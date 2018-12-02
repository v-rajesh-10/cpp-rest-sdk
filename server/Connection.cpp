#include "Connection.h"
#include "MailManager.h"
#include <cstdlib>
#include "../common/protocol.h"
#include "../common/Request.h"
#include "../common/Response.h"
#include "../common/pushDisableWarnings.h"
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include "../common/popDisableWarnings.h"

Connection::Connection(
  ServerConfig& config,
  MonitoringStorage& storage,
  MailManager& mailManager,
  boost::asio::ip::tcp::socket&& socket,
  std::function<void(Connection&)>&& onConnectionClose) :
  config(config),
  storage(storage),
  mailManager(mailManager),
  socket(std::move(socket)),
  onConnectionClose(std::move(onConnectionClose)) {
  receive();
}

void Connection::receive() {
  BOOST_LOG_TRIVIAL(trace) << "connection " << this << ", waiting for request";
  boost::asio::async_read_until(socket, data, "\r\n\r\n", [this](auto& error, auto bytes_transferred) { onReceive(error, bytes_transferred); });
  state = State::RECEIVING;
}

void Connection::processRequest(const Http::Request& request) {
  try {
    if (!request.hasPathParameter(PROTOCOL_USERKEY_PARAMETER_NAME)) {
      throw std::runtime_error(std::string("malformed request: mandatory parameter '") + PROTOCOL_USERKEY_PARAMETER_NAME + "' missing");
    }

    userKey = request.getPathParameterValue(PROTOCOL_USERKEY_PARAMETER_NAME);
    client = config.client(userKey);
    if (client == nullptr) {
      throw std::runtime_error("bad request: unknown user key '" + userKey);
    }

    if (!request.hasPathParameter(PROTOCOL_TIMESTAMP_PARAMETER_NAME)) {
      throw std::runtime_error(std::string("malformed request: mandatory parameter '") + PROTOCOL_TIMESTAMP_PARAMETER_NAME + "' missing");
    }

    auto timestamp = request.getPathParameterValue(PROTOCOL_TIMESTAMP_PARAMETER_NAME);
    if (timestamp != boost::posix_time::to_iso_string(boost::posix_time::from_iso_string(timestamp))) {
      throw std::runtime_error(std::string("malformed request: invalid '") + PROTOCOL_TIMESTAMP_PARAMETER_NAME + "' value '" + timestamp + "'");
    }

    MonitoringStorage::MonitoringRecord record;
    record.userKey = userKey;
    record.timestamp = timestamp;

    auto cpuUsageString = request.getPathParameterValue(PROTOCOL_CPU_USAGE_PARAMETER_NAME);
    if (cpuUsageString.empty()) {
      cpuUsagePresent = false;
      record.cpuLoad = "NULL";
    } else {
      cpuUsage = uint8_t(std::stoul(cpuUsageString));
      if (cpuUsageString != std::to_string(cpuUsage)) {
        throw std::runtime_error(std::string("malformed request: invalid '") + PROTOCOL_CPU_USAGE_PARAMETER_NAME + "' value '" + cpuUsageString + "'");
      }

      cpuUsagePresent = true;
      record.cpuLoad = cpuUsageString;
    }

    auto memoryUsageString = request.getPathParameterValue(PROTOCOL_MEMORY_USAGE_PARAMETER_NAME);
    if (memoryUsageString.empty()) {
      memoryUsagePresent = false;
      record.memoryUsage = "NULL";
    } else {
      memoryUsage = uint8_t(std::stoul(memoryUsageString));
      if (memoryUsageString != std::to_string(memoryUsage)) {
        throw std::runtime_error(std::string("malformed request: invalid '") + PROTOCOL_MEMORY_USAGE_PARAMETER_NAME + "' value '" + memoryUsageString + "'");
      }

      memoryUsagePresent = true;
      record.memoryUsage = memoryUsageString;
    }

    auto processCountString = request.getPathParameterValue(PROTOCOL_PROCESS_COUNT_PARAMETER_NAME);
    if (processCountString.empty()) {
      processCountPresent = false;
      record.processCount = "NULL";
    } else {
      processCount = uint32_t(std::stoul(processCountString));
      if (processCountString != std::to_string(processCount)) {
        throw std::runtime_error(std::string("malformed request: invalid '") + PROTOCOL_PROCESS_COUNT_PARAMETER_NAME + "' value '" + processCountString + "'");
      }

      processCountPresent = true;
      record.processCount = processCountString;
    }

    storageRequest = storage.storeRecord(
      record,
      [this]() { onStorageRequestSuccess(); },
      [this](std::string&& error) { onStorageRequestError(std::move(error)); });
    state = State::STORING_RECORD;
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(warning) << "connection " << this << ", " << e.what();
    Http::Response response;
    response.setStatus(Http::Response::BAD_REQUEST);
    std::ostream(&data) << response;
    send();
  }
}

void Connection::processLimits() {
  if (
    cpuUsagePresent && cpuUsage > client->cpuLoadLimit ||
    memoryUsagePresent && memoryUsage > client->memoryUsageLimit ||
    processCountPresent && processCount > client->processCountLimit) {
    storageRequest = storage.checkNotificationInterval(
      userKey,
      client->notificationPeriod,
      [this](bool shouldNotify0) { shouldNotify = shouldNotify0; onStorageRequestSuccess(); },
      [this](std::string&& error) { onStorageRequestError(std::move(error)); });
    state = State::CHECKING_NOTIFICATION;
    return;
  }

  sendSuccess();
}

void Connection::processNotification() {
  if (shouldNotify) {
    std::string limits;
    if (cpuUsagePresent && cpuUsage > client->cpuLoadLimit) {
      BOOST_LOG_TRIVIAL(info) << "user '" << userKey << "' exceeds CPU usage limit: " << uint32_t(cpuUsage) << '/' << uint32_t(client->cpuLoadLimit);
      limits = (boost::format(config.serverNotificationLimitLine()) %
                "CPU Usage" %
                (std::to_string(cpuUsage) + '%') %
                (std::to_string(client->cpuLoadLimit) + '%')).str() + '\n';
    }

    if (memoryUsagePresent && memoryUsage > client->memoryUsageLimit) {
      BOOST_LOG_TRIVIAL(info) << "user '" << userKey << "' exceeds memory usage limit: " << uint32_t(memoryUsage) << '/' << uint32_t(client->memoryUsageLimit);
      limits += (boost::format(config.serverNotificationLimitLine()) %
                 "Memory Usage" %
                 (std::to_string(memoryUsage) + '%') %
                 (std::to_string(client->memoryUsageLimit) + '%')).str() + '\n';
    }

    if (processCountPresent && processCount > client->processCountLimit) {
      BOOST_LOG_TRIVIAL(info) << "user '" << userKey << "' exceeds process count limit: " << processCount << '/' << client->processCountLimit;
      limits += (boost::format(config.serverNotificationLimitLine()) % "Process Count" % processCount % client->processCountLimit).str() + '\n';
    }

    mailManager.sendMail(
      std::string(config.serverNotificationSender()),
      std::string(client->mail),
      std::string(config.serverNotificationSubject()),
      (boost::format(config.serverNotificationBody()) % userKey % limits).str());

    storageRequest = storage.storeNotificationTime(
      userKey,
      [this]() { onStorageRequestSuccess(); },
      [this](std::string&& error) { onStorageRequestError(std::move(error)); });
    state = State::STORING_NOTIFICATION;
    return;
  }

  sendSuccess();
}

void Connection::send() {
  BOOST_LOG_TRIVIAL(trace) << "connection " << this << ", sending response";
  boost::asio::async_write(socket, data, [this](auto& error, auto bytes_transferred) { onSend(error, bytes_transferred); });
  state = State::SENDING;
}

void Connection::sendSuccess() {
  Http::Response response;
  response.setStatus(Http::Response::OK);
  std::ostream(&data) << response;
  send();
}

void Connection::onReceive(const boost::system::error_code& error, std::size_t) {
  assert(state == State::RECEIVING);
  try {
    if (error) {
      BOOST_LOG_TRIVIAL(warning) << "connection " << this << ", receive failed, " << error.message();
      destroy();
      return;
    }

    BOOST_LOG_TRIVIAL(trace) << "connection " << this << ", receive succeeded";
    Http::Request request;
    std::istream(&data) >> request;

    // apply business logic - go to the database, update it as needed and so on
    processRequest(request);
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(warning) << "connection " << this << ", " << e.what();
    destroy();
  }
}

void Connection::onStorageRequestSuccess() {
  assert(state == State::STORING_RECORD || state == State::CHECKING_NOTIFICATION || state == State::STORING_NOTIFICATION);
  try {
    if (state == State::STORING_RECORD) {
      processLimits();
    } else if (state == State::CHECKING_NOTIFICATION) {
      processNotification();
    } else {
      sendSuccess();
    }
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(warning) << "connection " << this << ", " << e.what();
    destroy();
  }
}

void Connection::onStorageRequestError(std::string&& error) {
  assert(state == State::STORING_RECORD || state == State::CHECKING_NOTIFICATION || state == State::STORING_NOTIFICATION);
  try {
    BOOST_LOG_TRIVIAL(warning) << "connection " << this << ", " << error;
    Http::Response response;
    response.setStatus(Http::Response::INTERNAL_SERVER_ERROR);
    std::ostream(&data) << response;
    send();
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(warning) << "connection " << this << ", " << e.what();
    destroy();
  }
}

void Connection::onSend(const boost::system::error_code& error, std::size_t) {
  assert(state == State::SENDING);
  try {
    if (error) {
      BOOST_LOG_TRIVIAL(warning) << "connection " << this << ", send failed, " << error.message();
      destroy();
      return;
    }

    BOOST_LOG_TRIVIAL(trace) << "connection " << this << ", send succeeded";
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    destroy();
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(warning) << "connection " << this << ", " << e.what();
    destroy();
  }
}

// notify Server and destroy Connection. Connection should not be used after calling this method.
void Connection::destroy() {
  onConnectionClose(*this);
  delete this;
}
