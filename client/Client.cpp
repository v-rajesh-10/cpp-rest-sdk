#include "Client.h"
#include "SystemMetrics.h"
#include "../common/protocol.h"
#include "../common/Request.h"
#include "../common/Response.h"
#include "../common/pushDisableWarnings.h"
#include <boost/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <fstream>
#include "../common/popDisableWarnings.h"

Client::Client(const std::string& configFile, SystemMetrics& systemMetrics) :
  systemMetrics(systemMetrics) {
  BOOST_LOG_TRIVIAL(info) << "loading configuration file \"" << configFile << '"';
  try {
    std::ifstream file(configFile);
    config = ClientConfig(file);
  } catch (std::exception& e) {
    throw std::runtime_error(std::string("unable to load configuration file \"") + configFile + "\", " + e.what());
  }

  BOOST_LOG_TRIVIAL(info) <<
    "loaded configuration, key=\"" << config.key() << '"' << ", updateInterval=" << config.updateInterval() <<
    ", host=\"" << config.host() << '"' << ", port=" << config.port();

  service = std::make_unique<boost::asio::io_service>();

  wait();
}

void Client::run() {
  service->run();
  if (exception != nullptr) {
    std::rethrow_exception(exception);
  }
}

void Client::wait() {
  BOOST_LOG_TRIVIAL(info) << "waiting for " << config.updateInterval() << " seconds";
  timer = std::make_unique<boost::asio::deadline_timer>(*service, boost::posix_time::seconds(long(config.updateInterval())));
  timer->async_wait([this](auto& error) { onWait(error); });
  state = State::WAITING;
}

void Client::resolve() {
  BOOST_LOG_TRIVIAL(info) << "resolving " << config.host() << ':' << config.port();
  resolver = std::make_unique<boost::asio::ip::tcp::resolver>(*service);
  resolver->async_resolve({config.host(), std::to_string(config.port())}, [this](auto& error, auto iterator) { onResolve(error, iterator); });
  state = State::RESOLVING;
}

void Client::connect(boost::asio::ip::tcp::resolver::iterator iterator) {
  BOOST_LOG_TRIVIAL(info) << "connecting";
  socket = std::make_unique<boost::asio::ip::tcp::socket>(*service);
  boost::asio::async_connect(*socket, iterator, [this](auto& error, auto iterator) { onConnect(error, iterator); });
  state = State::CONNECTING;
}

void Client::send() {
  BOOST_LOG_TRIVIAL(info) << "sending metrics";
  Http::Request request;
  request.setMethod(Http::Request::POST);
  request.setPath("/metrics");
  request.addPathParameter(PROTOCOL_USERKEY_PARAMETER_NAME, config.key());
  request.addPathParameter(PROTOCOL_TIMESTAMP_PARAMETER_NAME, boost::posix_time::to_iso_string(boost::posix_time::second_clock::universal_time()));
  request.addPathParameter(PROTOCOL_CPU_USAGE_PARAMETER_NAME, std::to_string(systemMetrics.queryCpuLoad()));
  request.addPathParameter(PROTOCOL_MEMORY_USAGE_PARAMETER_NAME, std::to_string(systemMetrics.queryMemoryUsage()));
  request.addPathParameter(PROTOCOL_PROCESS_COUNT_PARAMETER_NAME, std::to_string(systemMetrics.queryProcessCount()));
  std::ostream(&data) << request;
  boost::asio::async_write(*socket, data, [this](auto& error, auto bytes_transferred) { onSend(error, bytes_transferred); });
  state = State::SENDING;
}

void Client::receive() {
  BOOST_LOG_TRIVIAL(info) << "waiting for response";
  boost::asio::async_read_until(*socket, data, "\r\n\r\n", [this](auto& error, auto bytes_transferred) { onReceive(error, bytes_transferred); });
  state = State::RECEIVING;
}

void Client::onWait(const boost::system::error_code& error) {
  assert(state == State::WAITING);
  try {
    timer = nullptr;
    if (error) {
      BOOST_LOG_TRIVIAL(error) << "wait failed, " << error.message();
      service->stop();
      return;
    }

    resolve();
  } catch (...) {
    exception = std::current_exception();
    service->stop();
  }
}

void Client::onResolve(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator iterator) {
  assert(state == State::RESOLVING);
  try {
    if (error) {
      BOOST_LOG_TRIVIAL(warning) << "resolve failed, " << error.message();
      resolver = nullptr;
      wait();
      return;
    }

    BOOST_LOG_TRIVIAL(info) << "resolved to:";
    for (auto iterator2 = iterator; iterator2 != boost::asio::ip::tcp::resolver::iterator(); ++iterator2) {
      BOOST_LOG_TRIVIAL(info) << "  " << iterator2->endpoint().address().to_string() << ':' << std::to_string(iterator2->endpoint().port());
    }

    connect(iterator);
  } catch (...) {
    exception = std::current_exception();
    service->stop();
  }
}

void Client::onConnect(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator iterator) {
  assert(state == State::CONNECTING);
  try {
    if (error) {
      BOOST_LOG_TRIVIAL(warning) << "connect failed, " << error.message();
      resolver = nullptr;
      socket = nullptr;
      wait();
      return;
    }

    BOOST_LOG_TRIVIAL(info) << "connected to " << iterator->endpoint().address() << ':' << iterator->endpoint().port();
    resolver = nullptr;
    send();
  } catch (...) {
    exception = std::current_exception();
    service->stop();
  }
}

void Client::onSend(const boost::system::error_code& error, std::size_t) {
  assert(state == State::SENDING);
  try {
    if (error) {
      BOOST_LOG_TRIVIAL(warning) << "send failed, " << error.message();
      socket = nullptr;
      wait();
      return;
    }

    receive();
  } catch (...) {
    exception = std::current_exception();
    service->stop();
  }
}

void Client::onReceive(const boost::system::error_code& error, std::size_t) {
  assert(state == State::RECEIVING);
  try {
    if (error) {
      BOOST_LOG_TRIVIAL(warning) << "receive failed, " << error.message();
      socket = nullptr;
      wait();
      return;
    }

    Http::Response response;
    std::istream(&data) >> response;
    BOOST_LOG_TRIVIAL(info) << "response received, status=" << response.getStatus();

    boost::system::error_code result;
    socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, result);
    if (result) {
      BOOST_LOG_TRIVIAL(warning) << "shutdown failed, " << result.message();
    }

    socket = nullptr;
    wait();
  } catch (...) {
    exception = std::current_exception();
    service->stop();
  }
}
