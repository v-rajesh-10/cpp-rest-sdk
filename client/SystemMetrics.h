#pragma once

#include <cstdint>

class SystemMetrics {
public:
  virtual ~SystemMetrics() = default;
  virtual uint8_t queryCpuLoad() = 0;
  virtual uint8_t queryMemoryUsage() = 0;
  virtual uint32_t queryProcessCount() = 0;
};
