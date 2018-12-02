#include "Database.h"
#include "../common/pushDisableWarnings.h"
#include <boost/log/trivial.hpp>
#include <libpq-fe.h>
#include <winsock2.h>
#include "../common/popDisableWarnings.h"

DatabaseRequest::DatabaseRequest(
  Database& database0,
  const std::string& request0,
  std::function<void()>&& onSuccess0,
  std::function<void(std::string&&)>&& onError0) :
  database(database0),
  request(request0),
  onSuccess(std::move(onSuccess0)),
  onError(std::move(onError0)),
  previous(database0.lastRequest),
  next(nullptr),
  state(State::CREATED) {
  if (previous != nullptr) {
    assert(previous->next == nullptr);
    previous->next = this;
  } else {
    assert(database0.firstRequest == nullptr);
    database0.firstRequest = this;
  }

  database0.lastRequest = this;
}

DatabaseRequest::~DatabaseRequest() {
  assert(state != State::CONNECTED);
  for (auto result : results) {
    PQclear(result);
  }

  if (state != State::PROCESSED) {
    unlink();
    if (state == State::CONNECTING) {
      database.connectingConnections.push_back(connection);
    } else if (state == State::PROCESSING) {
      database.processingConnections.push_back(connection);
    }
  }
}

void DatabaseRequest::update(const char* connectionString) {
  assert(state == State::CREATED || state == State::CONNECTING || state == State::PROCESSING);
  if (state == State::CREATED) {
    if (database.connections.empty()) {
      connection = PQconnectStart(connectionString);
      if (connection == nullptr) {
        state = State::PROCESSED;
        unlink();
        std::function<void(std::string&&)> onError0 = std::move(onError);
        onError0(std::string("connection failed"));
        return;
      }

      if (PQstatus(connection) != CONNECTION_STARTED) {
        std::string error = std::string("PQconnectStart failed, ") + PQerrorMessage(connection);
        PQfinish(connection);
        state = State::PROCESSED;
        unlink();
        std::function<void(std::string&&)> onError0 = std::move(onError);
        onError0(std::move(error));
        return;
      }

      state = State::CONNECTING;
    } else {
      state = State::CONNECTED;
      connection = database.connections.back();
      database.connections.pop_back();
    }
  }

  if (state == State::CONNECTING) {
    SOCKET s = SOCKET(PQsocket(connection));
    fd_set reads{1, {s}};
    fd_set writes{1, {s}};
    fd_set errors{1, {s}};
    TIMEVAL timeout{0, 0};
    int result = select(0, &reads, &writes, &errors, &timeout);
    if (result == SOCKET_ERROR) {
      std::string error = std::string("PQconnectStart failed, ") + std::to_string(WSAGetLastError());
      PQfinish(connection);
      state = State::PROCESSED;
      unlink();
      std::function<void(std::string&&)> onError0 = std::move(onError);
      onError0(std::move(error));
      return;
    }

    if (result == 0) {
      return;
    }

    PostgresPollingStatusType pollingStatus = PQconnectPoll(connection);
    if (pollingStatus == PGRES_POLLING_READING || pollingStatus == PGRES_POLLING_WRITING) {
      return;
    }

    if (pollingStatus != PGRES_POLLING_OK) {
      std::string error = std::string("PQconnectStart failed, ") + PQerrorMessage(connection);
      PQfinish(connection);
      state = State::PROCESSED;
      unlink();
      std::function<void(std::string&&)> onError0 = std::move(onError);
      onError0(std::move(error));
      return;
    }

    state = State::CONNECTED;
  }

  if (state == State::CONNECTED) {
    if (PQsendQuery(connection, request.c_str()) != 1) {
      std::string error = std::string("PQsendQuery failed, ") + PQerrorMessage(connection);
      request = std::string();
      state = State::PROCESSED;
      database.connections.push_back(connection);
      unlink();
      std::function<void(std::string&&)> onError0 = std::move(onError);
      onError0(std::move(error));
      return;
    }

    request = std::string();
    state = State::PROCESSING;
  }

  if (PQconsumeInput(connection) != 1) {
    std::string error = std::string("PQconsumeInput failed, ") + PQerrorMessage(connection);
    PQfinish(connection);
    state = State::PROCESSED;
    unlink();
    std::function<void(std::string&&)> onError0 = std::move(onError);
    onError0(std::move(error));
    return;
  }

  if (PQisBusy(connection) == 1) {
    return;
  }

  PGresult* result = PQgetResult(connection);
  if (result != nullptr) {
    ExecStatusType status = PQresultStatus(result);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
      std::string error = std::string("PQsendQuery failed, ") + PQerrorMessage(connection);
      state = State::PROCESSED;
      database.processingConnections.push_back(connection);
      unlink();
      std::function<void(std::string&&)> onError0 = std::move(onError);
      onError0(std::move(error));
      return;
    }

    results.push_back(result);
  } else {
    state = State::PROCESSED;
    database.connections.push_back(connection);
    unlink();
    std::function<void()> onSuccess0 = std::move(onSuccess);
    onSuccess0();
  }
}

uint32_t DatabaseRequest::getResultCount() const {
  assert(state == State::PROCESSED);
  return uint32_t(results.size());
}

uint32_t DatabaseRequest::getResultTupleCount(uint32_t index) const {
  return uint32_t(PQntuples(results[index]));
}

std::string DatabaseRequest::getResult(uint32_t resultIndex, uint32_t tupleIndex, uint32_t fieldIndex) const {
  if (PQgetisnull(results[resultIndex], int32_t(tupleIndex), int32_t(fieldIndex))) {
    return{nullptr, 0};
  }

  return {
    PQgetvalue(results[resultIndex], int32_t(tupleIndex), int32_t(fieldIndex)),
    size_t(PQgetlength(results[resultIndex], int32_t(tupleIndex), int32_t(fieldIndex)))
  };
}

void DatabaseRequest::unlink() {
  if (previous != nullptr) {
    assert(previous->next == this);
    previous->next = next;
  } else {
    assert(database.firstRequest == this);
    database.firstRequest = next;
  }

  if (next != nullptr) {
    assert(next->previous == this);
    next->previous = previous;
  } else {
    assert(database.lastRequest == this);
    database.lastRequest = previous;
  }
}

Database::Database(std::string&& connectionString) :
  firstRequest(nullptr),
  lastRequest(nullptr),
  connectionString(std::move(connectionString)) {}

Database::~Database() {
  assert(firstRequest == nullptr);
  assert(lastRequest == nullptr);
  for (auto connection : connections) {
    PQfinish(connection);
  }

  for (auto connection : connectingConnections) {
    PQfinish(connection);
  }

  for (auto connection : processingConnections) {
    PQfinish(connection);
  }
}

std::unique_ptr<DatabaseRequest> Database::createRequest(
  const std::string& request,
  std::function<void()>&& onSuccess,
  std::function<void(std::string&&)>&& onError) {
  return std::make_unique<DatabaseRequest>(*this, request, std::move(onSuccess), std::move(onError));
}

void Database::update() {
  for (DatabaseRequest* request = firstRequest; request != nullptr;) {
    DatabaseRequest* next = request->next;
    request->update(connectionString.c_str());
    request = next;
  }

  uint32_t i = 0;
  for (uint32_t j = 0; j < connectingConnections.size(); ++j) {
    PGconn* connection = connectingConnections[j];
    SOCKET s = SOCKET(PQsocket(connection));
    fd_set reads{1, {s}};
    fd_set writes{1, {s}};
    fd_set errors{1, {s}};
    TIMEVAL timeout{0, 0};
    int result = select(0, &reads, &writes, &errors, &timeout);
    if (result == SOCKET_ERROR) {
      BOOST_LOG_TRIVIAL(error) << "PQconnectStart failed, " << WSAGetLastError();
      PQfinish(connection);
      continue;
    }

    if (result == 0) {
      connectingConnections[i] = connection;
      ++i;
      continue;
    }

    PostgresPollingStatusType pollingStatus = PQconnectPoll(connection);
    if (pollingStatus == PGRES_POLLING_READING || pollingStatus == PGRES_POLLING_WRITING) {
      connectingConnections[i] = connection;
      ++i;
      continue;
    }

    if (pollingStatus != PGRES_POLLING_OK) {
      BOOST_LOG_TRIVIAL(error) << "PQconnectStart failed, " << PQerrorMessage(connection);
      PQfinish(connection);
    } else {
      connections.push_back(connection);
    }
  }

  connectingConnections.resize(i);
  i = 0;
  for (uint32_t j = 0; j < processingConnections.size(); ++j) {
    PGconn* connection = processingConnections[j];
    if (PQconsumeInput(connection) != 1) {
      BOOST_LOG_TRIVIAL(error) << "PQconsumeInput failed, " << PQerrorMessage(connection);
      PQfinish(connection);
      continue;
    }

    if (PQisBusy(connection) == 1) {
      processingConnections[i] = connection;
      ++i;
      continue;
    }

    PGresult* result = PQgetResult(connection);
    if (result != nullptr) {
      processingConnections[i] = connection;
      ++i;
    } else {
      connections.push_back(connection);
    }
  }

  processingConnections.resize(i);
}
