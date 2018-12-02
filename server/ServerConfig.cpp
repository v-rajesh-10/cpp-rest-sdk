#include "ServerConfig.h"
#include "../common/pushDisableWarnings.h"
#include <boost/property_tree/xml_parser.hpp>
#include "../common/popDisableWarnings.h"

ServerConfig::ServerConfig(std::istream& source) {
  boost::property_tree::ptree pt;
  boost::property_tree::read_xml(source, pt);
  auto& configNode = pt.get_child(ROOT_ELEMENT);

  auto& databaseAttributes = configNode.get_child(DATABASE_ELEMENT).get_child("<xmlattr>");
  configDatabaseUser = databaseAttributes.get<std::string>(DATABASE_USER_ATTRIBUTE);
  configDatabasePassword = databaseAttributes.get<std::string>(DATABASE_PASSWORD_ATTRIBUTE);
  configDatabaseDbname = databaseAttributes.get<std::string>(DATABASE_DBNAME_ATTRIBUTE);
  configDatabaseHost = databaseAttributes.get<std::string>(DATABASE_HOST_ATTRIBUTE);
  configDatabasePort = databaseAttributes.get<std::string>(DATABASE_PORT_ATTRIBUTE);

  auto& serverAttributes = configNode.get_child(SERVER_ELEMENT).get_child("<xmlattr>");
  configServerAddress = serverAttributes.get<std::string>(SERVER_ADDRESS_ATTRIBUTE);
  auto serverPortString = serverAttributes.get<std::string>(SERVER_PORT_ATTRIBUTE);
  configServerPort = uint16_t(std::stoul(serverPortString));
  if (serverPortString != std::to_string(configServerPort)) {
    throw std::runtime_error("invalid port");
  }

  auto defaultNotificationPeriodString = serverAttributes.get<std::string>(SERVER_DEFAULT_NOTIFICATION_PERIOD_ATTRIBUTE);
  auto defaultNotificationPeriod = uint32_t(std::stoul(defaultNotificationPeriodString));
  if (defaultNotificationPeriodString != std::to_string(defaultNotificationPeriod)) {
    throw std::runtime_error("invalid default notification period");
  }

  configServerSmtpHost = serverAttributes.get<std::string>(SERVER_SMTP_HOST_ATTRIBUTE);
  auto serverSmtpPortString = serverAttributes.get<std::string>(SERVER_SMTP_PORT_ATTRIBUTE);
  configServerSmtpPort = uint16_t(std::stoul(serverSmtpPortString));
  if (serverSmtpPortString != std::to_string(configServerSmtpPort)) {
    throw std::runtime_error("invalid SMTP port");
  }

  configServerSmtpUser = serverAttributes.get<std::string>(SERVER_SMTP_USER_ATTRIBUTE);
  configServerSmtpPassword = serverAttributes.get<std::string>(SERVER_SMTP_PASSWORD_ATTRIBUTE);
  configServerNotificationSender = serverAttributes.get<std::string>(SERVER_NOTIFICATION_SENDER_ATTRIBUTE);
  configServerNotificationSubject = serverAttributes.get<std::string>(SERVER_NOTIFICATION_SUBJECT_ATTRIBUTE);
  configServerNotificationBody = serverAttributes.get<std::string>(SERVER_NOTIFICATION_BODY_ATTRIBUTE);
  configServerNotificationLimitLine = serverAttributes.get<std::string>(SERVER_NOTIFICATION_LIMIT_LINE_ATTRIBUTE);

  for (auto& clientNode : configNode) {
    if (clientNode.first != CLIENT_ELEMENT) {
      continue;
    }

    auto& clientAttributes = clientNode.second.get_child("<xmlattr>");
    auto key = clientAttributes.get<std::string>(CLIENT_KEY_ATTRIBUTE);
    if (!validateKey(key)) {
      throw std::runtime_error("invalid key \"" + key + '"');
    }

    if (configClients.count(key) != 0) {
      throw std::runtime_error("duplicate key \"" + key + '"');
    }

    auto mail = clientAttributes.get<std::string>(CLIENT_MAIL_ATTRIBUTE);

    auto notificationPeriodString = clientAttributes.get<std::string>(CLIENT_NOTIFICATION_PERIOD_ATTRIBUTE, defaultNotificationPeriodString);
    auto notificationPeriod = uint32_t(std::stoul(notificationPeriodString));
    if (notificationPeriodString != std::to_string(notificationPeriod)) {
      throw std::runtime_error("invalid notification period");
    }

    Client client{
      mail, // mail
      notificationPeriod, // notificationPeriod
      0, // cpuLoadLimit
      0, // memoryUsageLimit
      0}; // processCountLimit

    bool cpuLoadPresent = false;
    bool memoryUsagePresent = false;
    bool processCountPresent = false;
    for (auto& alertNode : clientNode.second) {
      if (alertNode.first != ALERT_ELEMENT) {
        continue;
      }

      auto& alertAttributes = alertNode.second.get_child("<xmlattr>");
      auto typeString = alertAttributes.get<std::string>(ALERT_TYPE_ATTRIBUTE);
      auto limitString = alertAttributes.get<std::string>(ALERT_LIMIT_ATTRIBUTE);
      uint32_t limit = uint32_t(std::stoul(limitString));
      if (typeString == ALERT_TYPE_CPU_LOAD) {
        if (cpuLoadPresent || limit > 100 || limitString != std::to_string(limit) + '%') {
          throw std::runtime_error("invalid cpu load alert limit");
        }

        client.cpuLoadLimit = uint8_t(limit);
        cpuLoadPresent = true;
      } else if (typeString == ALERT_TYPE_MEMORY_USAGE) {
        if (memoryUsagePresent || limit > 100 || limitString != std::to_string(limit) + '%') {
          throw std::runtime_error("invalid memory usage alert limit");
        }

        client.memoryUsageLimit = uint8_t(limit);
        memoryUsagePresent = true;
      } else if (typeString == ALERT_TYPE_PROCESS_COUNT) {
        if (processCountPresent || limitString != std::to_string(limit)) {
          throw std::runtime_error("invalid process count alert limit");
        }

        client.processCountLimit = limit;
        processCountPresent = true;
      } else {
        throw std::runtime_error("invalid alert type");
      }
    }

    if (!cpuLoadPresent) {
      throw std::runtime_error("missing cpu load alert");
    }

    if (!memoryUsagePresent) {
      throw std::runtime_error("missing memory usage alert");
    }

    if (!processCountPresent) {
      throw std::runtime_error("missing process count alert");
    }

    configClients.emplace(key, client);
  }
}

const std::string& ServerConfig::databaseUser() const {
  return configDatabaseUser;
}

const std::string& ServerConfig::databasePassword() const {
  return configDatabasePassword;
}

const std::string& ServerConfig::databaseDbname() const {
  return configDatabaseDbname;
}

const std::string& ServerConfig::databaseHost() const {
  return configDatabaseHost;
}

const std::string& ServerConfig::databasePort() const {
  return configDatabasePort;
}

const std::string& ServerConfig::serverAddress() const {
  return configServerAddress;
}

uint16_t ServerConfig::serverPort() const {
  return configServerPort;
}

const std::string& ServerConfig::serverSmtpHost() const {
  return configServerSmtpHost;
}

uint16_t ServerConfig::serverSmtpPort() const {
  return configServerSmtpPort;
}

const std::string& ServerConfig::serverSmtpUser() const {
  return configServerSmtpUser;
}

const std::string& ServerConfig::serverSmtpPassword() const {
  return configServerSmtpPassword;
}

const std::string& ServerConfig::serverNotificationSender() const {
  return configServerNotificationSender;
}

const std::string& ServerConfig::serverNotificationSubject() const {
  return configServerNotificationSubject;
}

const std::string& ServerConfig::serverNotificationBody() const {
  return configServerNotificationBody;
}

const std::string& ServerConfig::serverNotificationLimitLine() const {
  return configServerNotificationLimitLine;
}

const ServerConfig::Client* ServerConfig::client(const std::string& key) const {
  auto it = configClients.find(key);
  if (it != configClients.end()) {
    return &it->second;
  }

  return nullptr;
}

bool ServerConfig::validateKey(const std::string& key) {
  return !key.empty() && std::all_of(key.begin(), key.end(), [](auto c) {
    return c >= '0' && c <= '9' || c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c == '_';
  });
}
