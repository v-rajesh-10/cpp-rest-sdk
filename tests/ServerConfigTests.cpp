#include "../server/ServerConfig.h"
#include "../common/pushDisableWarnings.h"
#include <gtest/gtest.h>
#include "../common/popDisableWarnings.h"

TEST(ServerConfig, CheckSuccess) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,\n\nCollector Service has detected resource limit warning on your machine.\n%1%\nYours,\nCollector Service\n"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="user1" mail="user1@example.com" notificationPeriod="30">
    <alert type="cpu" limit="50%" />
    <alert type="memory" limit="80%" />
    <alert type="processes" limit="140" />
  </client>
  <client key="yyy" mail="yyy@asda.com">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_NO_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, CheckSuccess2) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="user1" mail="user1@example.com" notificationPeriod="30">
    <alert type="cpu" limit="50%" />
    <alert type="memory" limit="80%" />
    <alert type="processes" limit="140" />
  </client>
  <client key="yyy" mail="yyy@asda.com">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ServerConfig config(iss);
  ASSERT_EQ("smtp.gmail.com", config.serverSmtpHost());
  ASSERT_EQ(587, config.serverSmtpPort());
  ASSERT_EQ("a@a.com", config.serverSmtpUser());
  ASSERT_EQ("q1w2e3r4", config.serverSmtpPassword());
  ASSERT_EQ("collector@gtest.com", config.serverNotificationSender());
  ASSERT_EQ("Collector Service limit notification", config.serverNotificationSubject());
  ASSERT_EQ("Hello,\n\nCollector Service has detected resource limit warning on your machine.\n%1%\nYours,\nCollector Service\n", config.serverNotificationBody());
  ASSERT_EQ("%1%: detected value: %2%, limit: %3%", config.serverNotificationLimitLine());
  ASSERT_EQ("collector", config.databaseUser());
  ASSERT_EQ("collector", config.databaseDbname());
  ASSERT_EQ("q1w2e3r4", config.databasePassword());
  ASSERT_EQ("localhost", config.databaseHost());
  ASSERT_EQ("5432", config.databasePort());
  ASSERT_EQ("0.0.0.0", config.serverAddress());
  ASSERT_EQ(7700, config.serverPort());
  auto client1 = config.client("user1");
  ASSERT_NE(nullptr, client1);
  ASSERT_EQ("user1@example.com", client1->mail);
  ASSERT_EQ(50, client1->cpuLoadLimit);
  ASSERT_EQ(80, client1->memoryUsageLimit);
  ASSERT_EQ(140, client1->processCountLimit);
  ASSERT_EQ(30, client1->notificationPeriod);
  auto client2 = config.client("yyy");
  ASSERT_NE(nullptr, client2);
  ASSERT_EQ("yyy@asda.com", client2->mail);
  ASSERT_EQ(10, client2->cpuLoadLimit);
  ASSERT_EQ(40, client2->memoryUsageLimit);
  ASSERT_EQ(40, client2->processCountLimit);
  ASSERT_EQ(300, client2->notificationPeriod);
}

TEST(ServerConfig, MissingServer) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <client key="user1" mail="user1@example.com" notificationPeriod="30">
    <alert type="cpu" limit="50%" />
    <alert type="memory" limit="80%" />
    <alert type="processes" limit="140" />
  </client>
  <client key="yyy" mail="yyy@asda.com" notificationPeriod="30">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingDatabase) {
  std::istringstream iss(R"(
<config>
  <server address="0.0.0.0" port="7700" defaultNotificationPeriod="300"/>
  <client key="user1" mail="user1@example.com" notificationPeriod="30">
    <alert type="cpu" limit="50%" />
    <alert type="memory" limit="80%" />
    <alert type="processes" limit="140" />
  </client>
  <client key="yyy" mail="yyy@asda.com" notificationPeriod="30">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingClients) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_NO_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingDatabaseUser) {
  std::istringstream iss(R"(
<config>
  <database password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingDatabasePassword) {
  std::istringstream iss(R"(
<config>
  <database user="collector" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingDatabaseName) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingDatabaseHost) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingDatabasePort) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingServerAddress) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,\n\nCollector Service has detected resource limit warning on your machine.\n%1%\nYours,\nCollector Service\n"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingServerPort) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,\n\nCollector Service has detected resource limit warning on your machine.\n%1%\nYours,\nCollector Service\n"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MalformedSmtpHost) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587qqq"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingDefaultNotificationPeriod) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,\n\nCollector Service has detected resource limit warning on your machine.\n%1%\nYours,\nCollector Service\n"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingSmtpHost) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingSmtpPort) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingSmtpUser) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingSmtpPassword) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingSender) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingSubject) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingBody) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingLimitLine) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MalformedServerPort) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700qqqq"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MalformedDefaultNotificationPeriod) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300s"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, ExtraNodes1) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
<something />
)");

  ASSERT_NO_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, ExtraNodes2) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <something />
</config>
)");

  ASSERT_NO_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, ExtraNodes3) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="yyy" mail="yyy@asda.com" notificationPeriod="30">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
    <something />
  </client>
</config>
)");

  ASSERT_NO_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, ExtraAttributes1) {
  std::istringstream iss(R"(
<config extra="attrib">
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_NO_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, ExtraAttributes2) {
  std::istringstream iss(R"(
<config>
  <database extra="attrib" user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    extra="attrib"
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
</config>
)");

  ASSERT_NO_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, ExtraAttributes3) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client extra="attrib" key="yyy" mail="yyy@asda.com" notificationPeriod="30">
    <alert extra="attrib" type="cpu" limit="10%" />
    <alert extra="attrib" type="memory" limit="40%" />
    <alert extra="attrib" type="processes" limit="40" />
    <something />
  </client>
</config>
)");

  ASSERT_NO_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, AlertMissing) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="yyy" mail="yyy@asda.com" notificationPeriod="30">
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingKey) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client mail="yyy@asda.com" notificationPeriod="30">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MalformedKey) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="@" mail="yyy@asda.com" notificationPeriod="30">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, EmptyKey) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="" mail="yyy@asda.com" notificationPeriod="30">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingMail) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="xxx" notificationPeriod="30">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingNotificationPeriod) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="xxx" mail="yyy@asda.com">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_NO_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingType) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="xxx" mail="yyy@asda.com" notificationPeriod="30">
    <alert limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MissingLimit) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="xxx" mail="yyy@asda.com" notificationPeriod="30">
    <alert type="cpu" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MalformedLimit1) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="xxx" mail="yyy@asda.com" notificationPeriod="30">
    <alert type="cpu" limit="10d%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MalformedLimit2) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="xxx" mail="yyy@asda.com" notificationPeriod="30">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40e%" />
    <alert type="processes" limit="40" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, MalformedLimit3) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="xxx" mail="yyy@asda.com" notificationPeriod="30">
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40f" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}

TEST(ServerConfig, UnknownType) {
  std::istringstream iss(R"(
<config>
  <database user="collector" password="q1w2e3r4" dbname="collector" host="localhost" port="5432" />
  <server
    address="0.0.0.0"
    port="7700"
    defaultNotificationPeriod="300"
    smtpHost="smtp.gmail.com"
    smtpPort="587"
    smtpUser="a@a.com"
    smtpPassword="q1w2e3r4"
    notificationSender="collector@gtest.com"
    notificationSubject="Collector Service limit notification"
    notificationBody="Hello,&#10;&#10;Collector Service has detected resource limit warning on your machine.&#10;%1%&#10;Yours,&#10;Collector Service&#10;"
    notificationLimitLine="%1%: detected value: %2%, limit: %3%" />
  <client key="xxx" mail="yyy@asda.com" notificationPeriod="30">
    <alert type="fan_speed" limit="1200" />
    <alert type="cpu" limit="10%" />
    <alert type="memory" limit="40%" />
    <alert type="processes" limit="40f" />
  </client>
</config>
)");

  ASSERT_ANY_THROW(ServerConfig config(iss));
}
