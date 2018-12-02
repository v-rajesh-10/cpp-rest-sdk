#pragma once

#include "ClientConfig.h"
#include "../common/pushDisableWarnings.h"
#ifdef _MSC_VER
#define _WIN32_WINNT 0x0600
#endif
#include <boost/asio.hpp>
#include "../common/popDisableWarnings.h"

class SystemMetrics;

class Client {
public:
  Client(const std::string& configFile, SystemMetrics& systemMetrics);

  // Make sure object is never copied or moved
  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;

  void run();

private:
  enum class State {
    WAITING,
    RESOLVING,
    CONNECTING,
    SENDING,
    RECEIVING
  };

  State state;
  SystemMetrics& systemMetrics;
  ClientConfig config;
  std::unique_ptr<boost::asio::io_service> service;
  std::unique_ptr<boost::asio::deadline_timer> timer;
  std::unique_ptr<boost::asio::ip::tcp::resolver> resolver;
  std::unique_ptr<boost::asio::ip::tcp::socket> socket;
  boost::asio::streambuf data;
  std::exception_ptr exception;

  void wait();
  void resolve();
  void connect(boost::asio::ip::tcp::resolver::iterator iterator);
  void send();
  void receive();
  void onWait(const boost::system::error_code& error);
  void onResolve(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator iterator);
  void onConnect(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator iterator);
  void onSend(const boost::system::error_code& error, std::size_t bytes_transferred);
  void onReceive(const boost::system::error_code& error, std::size_t bytes_transferred);
};
