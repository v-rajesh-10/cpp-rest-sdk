#pragma once

#include "ServerConfig.h"
#include "../common/pushDisableWarnings.h"
#ifdef _MSC_VER
#define _WIN32_WINNT 0x0600
#endif
#include <boost/asio.hpp>
#include <set>
#include "../common/popDisableWarnings.h"

class Connection;
class MailManager;
class MonitoringStorage;

class Server {
public:
  explicit Server(const std::string& configFile);
  ~Server();

  // Make sure object is never copied or moved
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  void run();

private:
  constexpr static int TIMER_INTERVAL = 100; // in milliseconds

  ServerConfig config;
  std::unique_ptr<MonitoringStorage> storage;
  std::unique_ptr<MailManager> mailManager;
  boost::asio::io_service service;
  boost::asio::deadline_timer timer;
  boost::asio::ip::tcp::acceptor acceptor;
  boost::asio::ip::tcp::socket socket;
  boost::asio::ip::tcp::endpoint remoteEndpoint;
  std::set<Connection*> connections;
  std::exception_ptr exception;

  void accept();
  void onTimer(const boost::system::error_code& error);
  void onAccept(const boost::system::error_code& error);
  void onConnectionClose(Connection& connection);
};
