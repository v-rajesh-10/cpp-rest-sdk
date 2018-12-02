#pragma once

#include "../common/pushDisableWarnings.h"
#include <functional>
#include <memory>
#include <vector>
#include "../common/popDisableWarnings.h"

typedef struct pg_conn PGconn;
typedef struct pg_result PGresult;

class Database;

class DatabaseRequest {
public:
  DatabaseRequest(Database& database, const std::string& request, std::function<void()>&& onSuccess, std::function<void(std::string&&)>&& onError);
  ~DatabaseRequest();
  void update(const char* connectionString);
  uint32_t getResultCount() const;
  uint32_t getResultTupleCount(uint32_t index) const;
  std::string getResult(uint32_t resultIndex, uint32_t tupleIndex, uint32_t fieldIndex) const;

private:
  friend Database;

  enum class State {
    CREATED,
    CONNECTING,
    CONNECTED,
    PROCESSING,
    PROCESSED
  };

  Database& database;
  std::string request;
  std::function<void()> onSuccess;
  std::function<void(std::string&&)> onError;
  DatabaseRequest* previous;
  DatabaseRequest* next;
  State state;
  PGconn* connection;
  std::vector<PGresult*> results;

  DatabaseRequest(const DatabaseRequest&) = delete;
  DatabaseRequest& operator=(const DatabaseRequest&) = delete;
  void unlink();
};

class Database {
public:
  Database(std::string&& connectionString);
  ~Database();
  std::unique_ptr<DatabaseRequest> createRequest(const std::string& request, std::function<void()>&& onSuccess, std::function<void(std::string&&)>&& onError);
  void update();

private:
  friend DatabaseRequest;

  std::vector<PGconn*> connections;
  DatabaseRequest* firstRequest;
  DatabaseRequest* lastRequest;
  std::vector<PGconn*> connectingConnections;
  std::vector<PGconn*> processingConnections;
  std::string connectionString;

  Database(const Database&) = delete;
  Database& operator=(const Database&) = delete;
};
