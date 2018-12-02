#include "Server.h"
#include "../common/pushDisableWarnings.h"
#include <boost/log/trivial.hpp>
#include "../common/popDisableWarnings.h"

constexpr const char* CONFIG_FILE = "config.xml";

int main() {
  try {
    Server server(CONFIG_FILE);
    server.run();
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << e.what();
    return 1;
  }

  return 0;
}
