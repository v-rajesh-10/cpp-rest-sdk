#include "SystemMetrics.h"
#include "../common/pushDisableWarnings.h"
#include <algorithm>
#include <chrono>
#include <memory>
#include <random>
#include "../common/popDisableWarnings.h"

// "ultimate encapsulation pattern" - no includes, just declarations
std::unique_ptr<SystemMetrics> createTestSystemMetrics();

// Dummy SystemMetrics implementation
// Assigns random initial values to metrics and randomly mutates them on update interval
class TestSystemMetrics : public SystemMetrics {
public:
  TestSystemMetrics();
  ~TestSystemMetrics() override = default;

private:
  constexpr static int MINIMUM_CPU_LOAD = 0;
  constexpr static int MAXIMUM_CPU_LOAD = 100;
  constexpr static int CPU_LOAD_VOLATILITY = 5; // Maximal change for CPU load metric on each update
  constexpr static int MINIMUM_MEMORY_USAGE = 0;
  constexpr static int MAXIMUM_MEMORY_USAGE = 100;
  constexpr static int MEMORY_USAGE_VOLATILITY = 3; // Maximal change for memory metric on each update
  constexpr static int MINIMUM_PROCESS_COUNT = 8;
  constexpr static int MAXIMUM_PROCESS_COUNT = 50;
  constexpr static int PROCESS_COUNT_VOLATILITY = 1; // Maximal change for process count metric on each update
  constexpr static int UPDATE_INTERVAL = 100; // in milliseconds

  std::mt19937 generator;
  std::chrono::steady_clock::time_point updateTime;
  uint8_t cpuLoad;
  uint8_t memoryUsage;
  uint32_t processCount;

  uint8_t queryCpuLoad() override;
  uint8_t queryMemoryUsage() override;
  uint32_t queryProcessCount() override;
  void update();
  int mutate(int value, int volatility, int minimum, int maximum);
};

TestSystemMetrics::TestSystemMetrics() :
  generator(std::random_device()()),
  updateTime(std::chrono::steady_clock::now()),
  cpuLoad(uint8_t(std::uniform_int_distribution<>(MINIMUM_CPU_LOAD, MAXIMUM_CPU_LOAD)(generator))),
  memoryUsage(uint8_t(std::uniform_int_distribution<>(MINIMUM_MEMORY_USAGE, MAXIMUM_MEMORY_USAGE)(generator))),
  processCount(uint32_t(std::uniform_int_distribution<>(MINIMUM_PROCESS_COUNT, MAXIMUM_PROCESS_COUNT)(generator))) {}

uint8_t TestSystemMetrics::queryCpuLoad() {
  update(); // Conditionally update counters
  return cpuLoad;
}

uint8_t TestSystemMetrics::queryMemoryUsage() {
  update(); // Conditionally update counters
  return memoryUsage;
}

uint32_t TestSystemMetrics::queryProcessCount() {
  update(); // Conditionally update counters
  return processCount;
}

void TestSystemMetrics::update() {
  // Check if update duration has passed
  auto time = std::chrono::steady_clock::now();
  if (std::chrono::duration_cast<std::chrono::milliseconds>(time - updateTime).count() < UPDATE_INTERVAL) {
    return;
  }

  updateTime = time;

  // Randomly mutate metrics, clamping them to allowed range
  cpuLoad = uint8_t(mutate(int(cpuLoad), CPU_LOAD_VOLATILITY, MINIMUM_CPU_LOAD, MAXIMUM_CPU_LOAD));
  memoryUsage = uint8_t(mutate(int(memoryUsage), MEMORY_USAGE_VOLATILITY, MINIMUM_MEMORY_USAGE, MAXIMUM_MEMORY_USAGE));
  processCount = uint32_t(mutate(int(processCount), PROCESS_COUNT_VOLATILITY, MINIMUM_PROCESS_COUNT, MAXIMUM_PROCESS_COUNT));
}

int TestSystemMetrics::mutate(int value, int volatility, int minimum, int maximum) {
  value += std::uniform_int_distribution<>(-volatility, volatility)(generator); // Mutate by at most 'volatility' magnitude
  value = std::min(std::max(value, minimum), maximum); // Clamp to valid range
  return value;
}

std::unique_ptr<SystemMetrics> createTestSystemMetrics() {
  return std::make_unique<TestSystemMetrics>();
}
