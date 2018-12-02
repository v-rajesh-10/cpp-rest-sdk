#include "Client.h"
#include "SystemMetrics.h"
#include "../common/pushDisableWarnings.h"
#include <boost/log/trivial.hpp>
#include "../common/popDisableWarnings.h"

// "ultimate encapsulation pattern" - no includes, just declarations
std::unique_ptr<SystemMetrics> createPlatformSystemMetrics();
//std::unique_ptr<SystemMetrics> createTestSystemMetrics();

constexpr const char* CONFIG_FILE = "config.xml";

int main() {
  try {
    auto metrics = createPlatformSystemMetrics();
    Client client(CONFIG_FILE, *metrics);
    client.run();
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << e.what();
    return 1;
  }

  return 0;
}
