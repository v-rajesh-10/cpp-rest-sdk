#pragma once

#include "MonitoringStorage.h"
#include "ServerConfig.h"
#include "../common/pushDisableWarnings.h"
#ifdef _MSC_VER
#define _WIN32_WINNT 0x0600
#endif
#include <boost/asio.hpp>
#include "../common/popDisableWarnings.h"

namespace Http {

class Request;
class Response;

}

class MailManager;

class Connection {
public:
  Connection(
    ServerConfig& config,
    MonitoringStorage& storage,
    MailManager& mailManager,
    boost::asio::ip::tcp::socket&& socket,
    std::function<void(Connection&)>&& onConnectionClose);

  // Make sure object is never copied or moved
  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

private:
  enum class State {
    RECEIVING,
    STORING_RECORD,
    CHECKING_NOTIFICATION,
    STORING_NOTIFICATION,
    SENDING
  };

  State state;
  ServerConfig& config;
  MonitoringStorage& storage;
  MailManager& mailManager;
  boost::asio::ip::tcp::socket socket;
  std::function<void(Connection&)> onConnectionClose;
  boost::asio::streambuf data;
  std::string userKey;
  const ServerConfig::Client* client;
  bool cpuUsagePresent;
  uint8_t cpuUsage;
  bool memoryUsagePresent;
  uint8_t memoryUsage;
  bool processCountPresent;
  uint32_t processCount;
  std::unique_ptr<MonitoringStorage::Request> storageRequest;
  bool shouldNotify;

  void receive();
  void processRequest(const Http::Request& request);
  void processLimits();
  void processNotification();
  void send();
  void sendSuccess();
  void onReceive(const boost::system::error_code& error, std::size_t bytes_transferred);
  void onStorageRequestSuccess();
  void onStorageRequestError(std::string&& error);
  void onSend(const boost::system::error_code& error, std::size_t bytes_transferred);
  void destroy();
};
