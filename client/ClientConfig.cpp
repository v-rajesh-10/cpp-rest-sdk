#include "ClientConfig.h"
#include "../common/pushDisableWarnings.h"
#include <boost/property_tree/xml_parser.hpp>
#include "../common/popDisableWarnings.h"

ClientConfig::ClientConfig(std::istream& source) {
  boost::property_tree::ptree pt;
  boost::property_tree::read_xml(source, pt);
  auto& configNode = pt.get_child(ROOT_ELEMENT);

  auto& clientAttributes = configNode.get_child(CLIENT_ELEMENT).get_child("<xmlattr>");
  configKey = clientAttributes.get<std::string>(CLIENT_KEY_ATTRIBUTE);
  if (!validateKey(configKey)) {
    throw std::runtime_error("invalid key");
  }

  auto updateIntervalString = clientAttributes.get<std::string>(CLIENT_UPDATE_INTERVAL_ATTRIBUTE);
  configUpdateInterval = uint32_t(std::stoul(updateIntervalString));
  if (updateIntervalString != std::to_string(configUpdateInterval)) {
    throw std::runtime_error("invalid update interval");
  }

  auto& serverAttributes = configNode.get_child(SERVER_ELEMENT).get_child("<xmlattr>");
  configHost = serverAttributes.get<std::string>(SERVER_HOST_ATTRIBUTE);
  auto portString = serverAttributes.get<std::string>(SERVER_PORT_ATTRIBUTE);
  configPort = uint16_t(std::stoul(portString));
  if (portString != std::to_string(configPort)) {
    throw std::runtime_error("invalid port");
  }
}

const std::string& ClientConfig::key() const {
  return configKey;
}

uint32_t ClientConfig::updateInterval() const {
  return configUpdateInterval;
}

const std::string& ClientConfig::host() const {
  return configHost;
}

uint16_t ClientConfig::port() const {
  return configPort;
}

bool ClientConfig::validateKey(const std::string& key) {
  return !key.empty() && std::all_of(key.begin(), key.end(), [](auto c) {
    return c >= '0' && c <= '9' || c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c == '_';
  });
}
