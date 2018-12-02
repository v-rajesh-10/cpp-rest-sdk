#include "Database.h"
#include "MonitoringStorage.h"
#include "../common/pushDisableWarnings.h"
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include "../common/popDisableWarnings.h"

// "ultimate encapsulation pattern" - no includes, just declarations
std::unique_ptr<MonitoringStorage> createDatabaseMonitoringStorage(
  const std::string& username,
  const std::string& password,
  const std::string& dbname,
  const std::string& hostname,
  const std::string& port);

const char CHECK_NOTIFICATION_INTERVAL[] = R"(
SELECT to_char(last_notified, 'YYYYMMDD"T"HH24MISS') FROM public.notifications where user_key = '%1%';
)";

const char STORE_NOTIFICATION_TIME[] = R"(
INSERT INTO public.notifications
  (user_key, last_notified) VALUES('%1%', '%2%')
  ON CONFLICT(user_key) DO
    UPDATE SET last_notified = EXCLUDED.last_notified;
)";

const char STORE_RECORD[] = R"(
INSERT INTO public.statistics(
  user_key, time_stamp, cpu_usage, memory_usage, process_count)
  VALUES ('%1%', '%2%', %3%, %4%, %5%);
)";

class DatabaseMonitoringStorage : public MonitoringStorage {
public:
  DatabaseMonitoringStorage(
    const std::string& username,
    const std::string& password,
    const std::string& dbname,
    const std::string& hostname,
    const std::string& port);

  ~DatabaseMonitoringStorage() override = default;

  // Make sure object is never copied or moved
  DatabaseMonitoringStorage(const DatabaseMonitoringStorage&) = delete;
  DatabaseMonitoringStorage& operator=(const DatabaseMonitoringStorage&) = delete;

private:
  Database database;

  std::unique_ptr<Request> checkNotificationInterval(
    const std::string& userKey,
    uint32_t seconds,
    std::function<void(bool)>&& onSuccess,
    std::function<void(std::string&&)>&& onError) override;

  std::unique_ptr<Request> storeNotificationTime(
    const std::string& userKey,
    std::function<void()>&& onSuccess,
    std::function<void(std::string&&)>&& onError) override;


  std::unique_ptr<Request> storeRecord(
    const MonitoringRecord& data,
    std::function<void()>&& onSuccess,
    std::function<void(std::string&&)>&& onError) override;

  void update() override;

  static void onCheckNotificationIntervalComplete(DatabaseRequest& request, uint32_t seconds, std::function<void(bool)>& onSuccess);
};

DatabaseMonitoringStorage::DatabaseMonitoringStorage(
  const std::string& username,
  const std::string& password,
  const std::string& dbname,
  const std::string& hostname,
  const std::string& port) :
  database(std::string("dbname=") + dbname + " host=" + hostname + " password=" + password + " port=" + port + " user=" + username) {}

std::unique_ptr<MonitoringStorage::Request> DatabaseMonitoringStorage::checkNotificationInterval(
  const std::string& userKey,
  uint32_t seconds,
  std::function<void(bool)>&& onSuccess,
  std::function<void(std::string&&)>&& onError) {
  auto result = std::make_unique<Request>();
  auto request = result.get();

  auto lambda = [request, seconds, successHandler = std::move(onSuccess)]() mutable {
    onCheckNotificationIntervalComplete(*request->databaseRequest, seconds, successHandler);
  };

  result->databaseRequest = database.createRequest(
    (boost::format(CHECK_NOTIFICATION_INTERVAL) % userKey).str(),
    std::move(lambda),
    std::move(onError));

  return result;
}

std::unique_ptr<MonitoringStorage::Request> DatabaseMonitoringStorage::storeNotificationTime(
  const std::string& userKey,
  std::function<void()>&& onSuccess,
  std::function<void(std::string&&)>&& onError) {
  auto result = std::make_unique<Request>();
  auto now = boost::posix_time::to_iso_string(boost::posix_time::second_clock::universal_time());
  result->databaseRequest = database.createRequest(
    (boost::format(STORE_NOTIFICATION_TIME) % userKey % now).str(), std::move(onSuccess), std::move(onError));
  return result;
}

std::unique_ptr<MonitoringStorage::Request> DatabaseMonitoringStorage::storeRecord(
  const MonitoringRecord& data,
  std::function<void()>&& onSuccess,
  std::function<void(std::string&&)>&& onError) {
  auto result = std::make_unique<Request>();
  result->databaseRequest = database.createRequest(
    (boost::format(STORE_RECORD) %
      data.userKey % data.timestamp %
      data.cpuLoad %
      data.memoryUsage %
      data.processCount).str(),
    std::move(onSuccess),
    std::move(onError));
  return result;
}

void DatabaseMonitoringStorage::update() {
  database.update();
}

void DatabaseMonitoringStorage::onCheckNotificationIntervalComplete(DatabaseRequest& request, uint32_t seconds, std::function<void(bool)>& onSuccess) {
  assert(request.getResultCount() == 1);
  assert(request.getResultTupleCount(0) < 2);
  if (request.getResultTupleCount(0) == 0) {
    onSuccess(true);
    return;
  }

  auto lastNotified = boost::posix_time::from_iso_string(request.getResult(0, 0, 0));
  auto now = boost::posix_time::second_clock::universal_time();
  bool result = now >= lastNotified && now < (lastNotified + boost::posix_time::seconds(long(seconds)));
  onSuccess(!result);
}

std::unique_ptr<MonitoringStorage> createDatabaseMonitoringStorage(
  const std::string& username,
  const std::string& password,
  const std::string& dbname,
  const std::string& hostname,
  const std::string& port) {
  return std::make_unique<DatabaseMonitoringStorage>(username, password, dbname, hostname, port);
}
