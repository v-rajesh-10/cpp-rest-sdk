#pragma once

#include "../common/pushDisableWarnings.h"
#include <map>
#include "../common/popDisableWarnings.h"

class ServerConfig {
public:
  struct Client {
    std::string mail;
    uint32_t notificationPeriod;
    uint8_t cpuLoadLimit;
    uint8_t memoryUsageLimit;
    uint32_t processCountLimit;
  };

  ServerConfig() = default;

  // Load server configuration from file
  // Throw std::runtime_error if there was an error reading file or parsing configuration
  ServerConfig(std::istream& source);

  // Make sure object is never copied
  ServerConfig(const ServerConfig&) = delete;
  ServerConfig(ServerConfig&&) = default;
  ServerConfig& operator=(const ServerConfig&) = delete;
  ServerConfig& operator=(ServerConfig&&) = default;

  const std::string& databaseUser() const;
  const std::string& databasePassword() const;
  const std::string& databaseDbname() const;
  const std::string& databaseHost() const;
  const std::string& databasePort() const;
  const std::string& serverAddress() const;
  uint16_t serverPort() const;
  const std::string& serverSmtpHost() const;
  uint16_t serverSmtpPort() const;
  const std::string& serverSmtpUser() const;
  const std::string& serverSmtpPassword() const;
  const std::string& serverNotificationSender() const;
  const std::string& serverNotificationSubject() const;
  const std::string& serverNotificationBody() const;
  const std::string& serverNotificationLimitLine() const;
  const Client* client(const std::string& key) const;

private:
  constexpr static const char* ROOT_ELEMENT = "config";
  constexpr static const char* DATABASE_ELEMENT = "database";
  constexpr static const char* DATABASE_USER_ATTRIBUTE = "user";
  constexpr static const char* DATABASE_PASSWORD_ATTRIBUTE = "password";
  constexpr static const char* DATABASE_DBNAME_ATTRIBUTE = "dbname";
  constexpr static const char* DATABASE_HOST_ATTRIBUTE = "host";
  constexpr static const char* DATABASE_PORT_ATTRIBUTE = "port";
  constexpr static const char* SERVER_ELEMENT = "server";
  constexpr static const char* SERVER_ADDRESS_ATTRIBUTE = "address";
  constexpr static const char* SERVER_PORT_ATTRIBUTE = "port";
  constexpr static const char* SERVER_DEFAULT_NOTIFICATION_PERIOD_ATTRIBUTE = "defaultNotificationPeriod";
  constexpr static const char* SERVER_SMTP_HOST_ATTRIBUTE = "smtpHost";
  constexpr static const char* SERVER_SMTP_PORT_ATTRIBUTE = "smtpPort";
  constexpr static const char* SERVER_SMTP_USER_ATTRIBUTE = "smtpUser";
  constexpr static const char* SERVER_SMTP_PASSWORD_ATTRIBUTE = "smtpPassword";
  constexpr static const char* SERVER_NOTIFICATION_SENDER_ATTRIBUTE = "notificationSender";
  constexpr static const char* SERVER_NOTIFICATION_SUBJECT_ATTRIBUTE = "notificationSubject";
  constexpr static const char* SERVER_NOTIFICATION_BODY_ATTRIBUTE = "notificationBody";
  constexpr static const char* SERVER_NOTIFICATION_LIMIT_LINE_ATTRIBUTE = "notificationLimitLine";
  constexpr static const char* CLIENT_ELEMENT = "client";
  constexpr static const char* CLIENT_KEY_ATTRIBUTE = "key";
  constexpr static const char* CLIENT_MAIL_ATTRIBUTE = "mail";
  constexpr static const char* CLIENT_NOTIFICATION_PERIOD_ATTRIBUTE = "notificationPeriod";
  constexpr static const char* ALERT_ELEMENT = "alert";
  constexpr static const char* ALERT_TYPE_ATTRIBUTE = "type";
  constexpr static const char* ALERT_LIMIT_ATTRIBUTE = "limit";
  constexpr static const char* ALERT_TYPE_CPU_LOAD = "cpu";
  constexpr static const char* ALERT_TYPE_MEMORY_USAGE = "memory";
  constexpr static const char* ALERT_TYPE_PROCESS_COUNT = "processes";

  std::string configDatabaseUser;
  std::string configDatabasePassword;
  std::string configDatabaseDbname;
  std::string configDatabaseHost;
  std::string configDatabasePort;
  std::string configServerAddress;
  uint16_t configServerPort;
  std::string configServerSmtpHost;
  uint16_t configServerSmtpPort;
  std::string configServerSmtpUser;
  std::string configServerSmtpPassword;
  std::string configServerNotificationSender;
  std::string configServerNotificationSubject;
  std::string configServerNotificationBody;
  std::string configServerNotificationLimitLine;
  std::map<std::string, Client> configClients;

  static bool validateKey(const std::string& key);
};
