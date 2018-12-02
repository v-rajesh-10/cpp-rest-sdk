#include "../client/ClientConfig.h"
#include "../common/pushDisableWarnings.h"
#include <gtest/gtest.h>
#include "../common/popDisableWarnings.h"

TEST(ClientConfig, CheckSuccess) {
  std::istringstream iss(R"(
<config>
  <client key="yyy" updateInterval="3" />
  <server host="localhost" port="7700" />
</config>
)");

  ASSERT_NO_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, CheckSuccess2) {
  std::istringstream iss(R"(
<config>
  <client key="xxx" updateInterval="300" />
  <server host="localhost" port="7700" />
</config>
)");

  ClientConfig config(iss);

  ASSERT_EQ("xxx", config.key());
  ASSERT_EQ("localhost", config.host());
  ASSERT_EQ(300, config.updateInterval());
  ASSERT_EQ(7700, config.port());
}

TEST(ClientConfig, MissingServer) {
  std::istringstream iss(R"(
<config>
  <client key="xxx" updateInterval="300" />
</config>
)");

  ASSERT_ANY_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, MalformedServer) {
  std::istringstream iss(R"(
<config>
  <client key="xxx" updateInterval="300" />
  <server host___="localhost" port="7700"/>
</config>
)");

  ASSERT_ANY_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, MissingClient) {
  std::istringstream iss(R"(
<config>
  <server host="localhost" port="7700"/>
</config>
)");

  ASSERT_ANY_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, MalformedClient) {
  std::istringstream iss(R"(
<config>
  <client key___="xxx" updateInterval="300" />
  <server host="localhost" port="7700" />
</config>
)");

  ASSERT_ANY_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, MalformedClient2) {
  std::istringstream iss(R"(
<config>
  <client key="xxx" updateInterval___="300" />
  <server host="localhost" port="7700"/>
</config>
)");

  ASSERT_ANY_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, MalformedUpdateInterval) {
  std::istringstream iss(R"(
<config>
  <client key="xxx" updateInterval="300ms" />
  <server host="localhost" port="7700"/>
</config>
)");

  ASSERT_ANY_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, MalformedKey) {
  std::istringstream iss(R"(
<config>
  <client key="aaa!aa" updateInterval="300" />
  <server host="localhost" port="7700"/>
</config>
)");

  ASSERT_ANY_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, MalformedKey2) {
  std::istringstream iss(R"(
<config>
  <client key="xx@xxx" updateInterval="300" />
  <server host="localhost" port="7700"/>
</config>
)");

  ASSERT_ANY_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, EmptyKey) {
  std::istringstream iss(R"(
<config>
  <client key="" updateInterval="300" />
  <server host="localhost" port="7700"/>
</config>
)");

  ASSERT_ANY_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, MalformedPort) {
  std::istringstream iss(R"(
<config>
  <client key="xxx" updateInterval="300" />
  <server host="localhost" port="7700q"/>
</config>
)");

  ASSERT_ANY_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, MalformedConfig) {
  std::istringstream iss(R"(
Hi there!
)");

  ASSERT_ANY_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, ExtraNodes) {
  std::istringstream iss(R"(
<config>
  <client key="xxx" updateInterval="300" />
  <server host="localhost" port="7700"/>
</config>
<something/>
)");

  ASSERT_NO_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, ExtraNodes2) {
  std::istringstream iss(R"(
<config>
  <client key="xxx" updateInterval="300" />
  <server host="localhost" port="7700"/>
  <something/>
</config>
)");

  ASSERT_NO_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, ExtraAttributes) {
  std::istringstream iss(R"(
<config>
  <client key="xxx" updateInterval="300" favorite-colour="chartreuse"/>
  <server host="localhost" port="7700"/>
</config>
)");

  ASSERT_NO_THROW(ClientConfig config(iss));
}

TEST(ClientConfig, ExtraAttributes2) {
  std::istringstream iss(R"(
<config>
  <client key="xxx" updateInterval="300"/>
  <server host="localhost" port="7700" favorite-animal="ostrich"/>
</config>
)");

  ASSERT_NO_THROW(ClientConfig config(iss));
}
