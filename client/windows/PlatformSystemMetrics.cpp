#include "../SystemMetrics.h"
#include "../../common/pushDisableWarnings.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <pdh.h>
#include "../../common/popDisableWarnings.h"

// "ultimate encapsulation pattern" - no includes, just declarations
std::unique_ptr<SystemMetrics> createPlatformSystemMetrics();

// PlatformSystemMetrics for Windows
// Uses Windows performance counters for CPU load and process count information
// Uses GlobalMemoryStatusEx for memory usage information
class PlatformSystemMetrics : public SystemMetrics {
public:
  PlatformSystemMetrics();
  ~PlatformSystemMetrics() override;

private:
  constexpr static int UPDATE_INTERVAL = 100; // in milliseconds

  PDH_HQUERY query;
  PDH_HCOUNTER cpuLoadCounter;
  PDH_HCOUNTER processCountCounter;
  std::chrono::steady_clock::time_point updateTime;
  uint8_t cpuLoad;
  uint8_t memoryUsage;
  uint32_t processCount;

  uint8_t queryCpuLoad() override;
  uint8_t queryMemoryUsage() override;
  uint32_t queryProcessCount() override;
  void update();
  void closeQuery();
};

PlatformSystemMetrics::PlatformSystemMetrics() {
  // Initialize performance counters query
  PDH_STATUS result = PdhOpenQuery(nullptr, 0, &query);
  if (result != ERROR_SUCCESS) {
    throw std::runtime_error("PdhOpenQuery failed, result=" + std::to_string(result));
  }

  // Add counter for CPU load
  result = PdhAddCounter(query, L"\\Processor(_Total)\\% Processor Time", 0, &cpuLoadCounter);
  if (result != ERROR_SUCCESS) {
    closeQuery();
    throw std::runtime_error("PdhAddCounter failed, result=" + std::to_string(result));
  }

  // Add counter for process count
  result = PdhAddCounter(query, L"\\System\\Processes", 0, &processCountCounter);
  if (result != ERROR_SUCCESS) {
    closeQuery();
    throw std::runtime_error("PdhAddCounter failed, result=" + std::to_string(result));
  }

  // Collect counter information once, this is required for PdhGetFormattedCounterValue, which needs two samples to work correctly
  result = PdhCollectQueryData(query);
  if (result != ERROR_SUCCESS) {
    closeQuery();
    throw std::runtime_error("PdhCollectQueryData failed, result=" + std::to_string(result));
  }

  // Set update time in the past, to force counter collection even if queries immediately follow initialization
  // This is required for PdhGetFormattedCounterValue, which needs two samples to work correctly
  updateTime = std::chrono::steady_clock::now() - std::chrono::milliseconds(UPDATE_INTERVAL);
  cpuLoad = 0; // Sometimes users see this value, PlatformSystemMetrics::queryCpuLoad() commentary explains why
  memoryUsage = 0; // This value should never be used
  processCount = 0; // This value should never be used
}

PlatformSystemMetrics::~PlatformSystemMetrics() {
  closeQuery();
}

// This method returns CPU load, in percents. Or at least attempts to do so.
// Sometimes, PdhGetFormattedCounterValue fails on CPU load counter, cpuLoad variable does not change and the previous value is returned
// This may happen, for example, when queryCpuLoad() is called immediately after PlatformSystemMetrics::PlatformSystemMetrics() -
// in this case queryCpuLoad() will return initial value for cpuLoad variable, 0
uint8_t PlatformSystemMetrics::queryCpuLoad() {
  update(); // Conditionally update counters
  return cpuLoad;
}

uint8_t PlatformSystemMetrics::queryMemoryUsage() {
  update(); // Conditionally update counters
  return memoryUsage;
}

uint32_t PlatformSystemMetrics::queryProcessCount() {
  update(); // Conditionally update counters
  return processCount;
}

void PlatformSystemMetrics::update() {
  // Check if update duration has passed
  // Should be also triggered when queries are made for the first time, see PlatformSystemMetrics::PlatformSystemMetrics()
  auto time = std::chrono::steady_clock::now();
  if (std::chrono::duration_cast<std::chrono::milliseconds>(time - updateTime).count() < UPDATE_INTERVAL) {
    return;
  }

  updateTime = time;

  // Collect counter information
  PDH_STATUS result = PdhCollectQueryData(query);
  if (result != ERROR_SUCCESS) {
    throw std::runtime_error("PdhCollectQueryData failed, result=" + std::to_string(result));
  }

  // Read CPU load counter. Sometimes this call fails, cpuLoad variable is left intact in such cases.
  PDH_FMT_COUNTERVALUE value;
  result = PdhGetFormattedCounterValue(cpuLoadCounter, PDH_FMT_LONG, nullptr, &value);
  if (result == ERROR_SUCCESS) {
    cpuLoad = uint8_t(value.longValue);
  }

  // Read process count counter
  result = PdhGetFormattedCounterValue(processCountCounter, PDH_FMT_LONG, nullptr, &value);
  if (result != ERROR_SUCCESS) {
    throw std::runtime_error("PdhGetFormattedCounterValue failed, result=" + std::to_string(result));
  }

  processCount = uint32_t(value.longValue);

  // Read memory usage
  MEMORYSTATUSEX status;
  status.dwLength = sizeof status;
  if (GlobalMemoryStatusEx(&status) != TRUE) {
    throw std::runtime_error("GlobalMemoryStatusEx failed, result=" + std::to_string(GetLastError()));
  }

  memoryUsage = uint8_t(status.dwMemoryLoad);
}

void PlatformSystemMetrics::closeQuery() {
  // Finalize query, also finalizing counters
  PDH_STATUS result = PdhCloseQuery(query);
  if (result != ERROR_SUCCESS) {
    // This means something went really wrong. There's no good way to handle this. Let's log the problem and perform a suicide.
    std::cerr << "PdhCloseQuery failed, result=" << std::to_string(result) << std::endl;
    abort();
  }
}

std::unique_ptr<SystemMetrics> createPlatformSystemMetrics() {
  return std::make_unique<PlatformSystemMetrics>();
}
