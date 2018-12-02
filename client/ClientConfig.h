#pragma once

#include "../common/pushDisableWarnings.h"
#include <string>
#include "../common/popDisableWarnings.h"

class ClientConfig {
public:
  ClientConfig() = default;

  // Load client configuration from file
  // Throw std::runtime_error if there was an error reading file or parsing configuration
  ClientConfig(std::istream& source);

  const std::string& key() const;
  uint32_t updateInterval() const;
  const std::string& host() const;
  uint16_t port() const;

private:
  constexpr static const char* ROOT_ELEMENT = "config";
  constexpr static const char* CLIENT_ELEMENT = "client";
  constexpr static const char* CLIENT_KEY_ATTRIBUTE = "key";
  constexpr static const char* CLIENT_UPDATE_INTERVAL_ATTRIBUTE = "updateInterval";
  constexpr static const char* SERVER_ELEMENT = "server";
  constexpr static const char* SERVER_HOST_ATTRIBUTE = "host";
  constexpr static const char* SERVER_PORT_ATTRIBUTE = "port";

  std::string configKey;
  uint32_t configUpdateInterval;
  std::string configHost;
  uint16_t configPort;

  static bool validateKey(const std::string& key);
};
