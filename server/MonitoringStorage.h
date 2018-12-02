#pragma once

#include "../common/pushDisableWarnings.h"
#include <functional>
#include <memory>
#include "../common/popDisableWarnings.h"

class DatabaseRequest;

class MonitoringStorage {
public:
  struct MonitoringRecord {
    std::string userKey;
    std::string timestamp;
    std::string cpuLoad;
    std::string memoryUsage;
    std::string processCount;
  };

  struct Request {
    Request() = default;
    ~Request();

    // Make sure object is never copied or moved
    Request(const Request&) = delete;
    Request& operator=(const Request&) = delete;

    std::unique_ptr<DatabaseRequest> databaseRequest;
  };

  virtual ~MonitoringStorage() = default;

  virtual std::unique_ptr<Request> checkNotificationInterval(
    const std::string& userKey,
    uint32_t seconds,
    std::function<void(bool)>&& onSuccess,
    std::function<void(std::string&&)>&& onError) = 0;

  virtual std::unique_ptr<Request> storeNotificationTime(
    const std::string& userKey,
    std::function<void()>&& onSuccess,
    std::function<void(std::string&&)>&& onError) = 0;

  virtual std::unique_ptr<Request> storeRecord(
    const MonitoringRecord& data,
    std::function<void()>&& onSuccess,
    std::function<void(std::string&&)>&& onError) = 0;

  virtual void update() = 0;
};
