#include "Server.h"
#include "Connection.h"
#include "MailManager.h"
#include "MonitoringStorage.h"
#include "../common/pushDisableWarnings.h"
#include <boost/log/trivial.hpp>
#include <fstream>
#include "../common/popDisableWarnings.h"

// "ultimate encapsulation pattern" - no includes, just declarations
std::unique_ptr<MonitoringStorage> createDatabaseMonitoringStorage(
  const std::string& username,
  const std::string& password,
  const std::string& dbname,
  const std::string& hostname,
  const std::string& port);

std::unique_ptr<MailManager> createCSmtpMailManager(const std::string& host, uint16_t port, const std::string& user, const std::string& password);

Server::Server(const std::string& configFile) : timer(service), acceptor(service), socket(service) {
  BOOST_LOG_TRIVIAL(info) << "loading configuration file \"" << configFile << '"';
  try {
    std::ifstream file(configFile);
    config = ServerConfig(file);
  } catch (std::exception& e) {
    throw std::runtime_error(std::string("unable to load configuration file \"") + configFile + "\", " + e.what());
  }

  BOOST_LOG_TRIVIAL(info) << "loaded configuration, address=" << config.serverAddress() << ", port=" << config.serverPort();

  storage = createDatabaseMonitoringStorage(
    config.databaseUser(), config.databasePassword(), config.databaseDbname(), config.databaseHost(), config.databasePort());

  mailManager = createCSmtpMailManager(config.serverSmtpHost(), config.serverSmtpPort(), config.serverSmtpUser(), config.serverSmtpPassword());

  boost::asio::ip::tcp::resolver resolver(service);
  auto localEndpoint = resolver.resolve({config.serverAddress(), std::to_string(config.serverPort())})->endpoint();

  acceptor.open(localEndpoint.protocol());
  acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor.bind(localEndpoint);
  acceptor.listen();

  timer.expires_from_now(boost::posix_time::milliseconds(TIMER_INTERVAL));
  timer.async_wait([this](auto& error) { onTimer(error); });
  accept();
}

Server::~Server() {
  // exception-safe way to delete all connections
  // this should never lead to a call to onConnectionClose(Connection&)
  for (auto it = connections.begin(); it != connections.end(); ++it) {
    delete *it;
  }
}

void Server::run() {
  service.run();
  if (exception != nullptr) {
    std::rethrow_exception(exception);
  }
}

void Server::accept() {
  BOOST_LOG_TRIVIAL(info) << "waiting for connection";
  acceptor.async_accept(socket, remoteEndpoint, [this](auto& error) { onAccept(error); });
}

void Server::onTimer(const boost::system::error_code& error) {
  try {
    if (error) {
      throw std::runtime_error("timer failed, " + error.message());
    }

    storage->update();
    timer.expires_from_now(boost::posix_time::milliseconds(TIMER_INTERVAL));
    timer.async_wait([this](auto& error) { onTimer(error); });
  } catch (...) {
    exception = std::current_exception();
    service.stop();
  }
}

void Server::onAccept(const boost::system::error_code& error) {
  try {
    if (error) {
      BOOST_LOG_TRIVIAL(warning) << "accept failed, " << error.message();
      accept();
      return;
    }

    BOOST_LOG_TRIVIAL(info) << "client connected from " << remoteEndpoint.address() << ':' << remoteEndpoint.port();

    // create exception-safe pointer to new Connection
    auto connection = std::make_unique<Connection>(config, *storage, *mailManager, std::move(socket), [this](auto& connection) { onConnectionClose(connection); });

    // insert Connection pointer into connections container
    // this operation can throw, in which case std::unique_pointer<Connection> will gracefully destroy Connection object
    connections.insert(&*connection);

    // now we can safely release std::unique_pointer<Connection>
    connection.release();

    accept();
  } catch (...) {
    exception = std::current_exception();
    service.stop();
  }
}

// called by Connection to notify Server that Connection will destroy itself
void Server::onConnectionClose(Connection& connection) {
  assert(connections.count(&connection) == 1);
  connections.erase(&connection);
}
