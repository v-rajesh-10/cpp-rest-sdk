#include "../common/Request.h"
#include "../common/Response.h"
#include "../common/pushDisableWarnings.h"
#include <gtest/gtest.h>
#include "../common/popDisableWarnings.h"

struct RequestTest : public ::testing::Test {
  void SetUp() override {
    request1 = Http::Request();
    request2 = Http::Request();
    sstream.clear();
  }

  Http::Request request1;
  Http::Request request2;
  std::stringstream sstream;

  RequestTest() = default;
  RequestTest(const RequestTest&) = delete;
  RequestTest& operator=(const RequestTest&) = delete;
};

TEST_F(RequestTest, CheckMethod) {
  request1.setMethod(Http::Request::POST);
  sstream << request1;
  sstream >> request2;
  ASSERT_EQ(Http::Request::POST, request2.getMethod());
}

TEST_F(RequestTest, CheckPath) {
  request1.setMethod(Http::Request::POST).setPath("/index.html");
  sstream << request1;
  sstream >> request2;
  ASSERT_EQ("/index.html", request2.getPath());
}

TEST_F(RequestTest, CheckParametrizedPath) {
  request1.setMethod(Http::Request::POST).setPath("/index.html").addPathParameter("name1", "value1").addPathParameter("name2", "value2");
  sstream << request1;
  sstream >> request2;
  ASSERT_EQ("/index.html", request2.getPath());
  ASSERT_EQ(2, request2.getPathParameterCount());
  ASSERT_EQ("name1", request2.getPathParameterName(0));
  ASSERT_EQ("name2", request2.getPathParameterName(1));
  ASSERT_EQ("value1", request2.getPathParameterValue(0));
  ASSERT_EQ("value2", request2.getPathParameterValue(1));
  ASSERT_EQ("value1", request2.getPathParameterValue("name1"));
  ASSERT_EQ("value2", request2.getPathParameterValue("name2"));
  ASSERT_EQ(true, request2.hasPathParameter("name1"));
  ASSERT_EQ(false, request2.hasPathParameter("name3"));
}

TEST_F(RequestTest, CheckFields) {
  request1.setMethod(Http::Request::POST).setPath("/index.html").addField("name1", "value1").addField("name2", "value2");
  sstream << request1;
  sstream >> request2;
  ASSERT_EQ("/index.html", request2.getPath());
  ASSERT_EQ(2, request2.getFieldCount());
  ASSERT_EQ("name1", request2.getFieldName(0));
  ASSERT_EQ("name2", request2.getFieldName(1));
  ASSERT_EQ("value1", request2.getFieldValue(0));
  ASSERT_EQ("value2", request2.getFieldValue(1));
  ASSERT_EQ("value1", request2.getFieldValue("name1"));
  ASSERT_EQ("value2", request2.getFieldValue("name2"));
  ASSERT_EQ(true, request2.hasField("name1"));
  ASSERT_EQ(false, request2.hasField("name3"));
}

TEST_F(RequestTest, CheckString) {
  request1.setMethod(Http::Request::POST).setPath("/index.html")
    .addField("name1", "value1").addField("name2", "value2")
    .addPathParameter("name1", "value1").addPathParameter("name2", "value2");
  sstream << request1;
  sstream >> request2;
  ASSERT_EQ(request1.toString(), request2.toString());
  Http::Request request3;
  request3.fromString(request1.toString());
  ASSERT_EQ(request3.toString(), request2.toString());
}

struct ResponseTest : public ::testing::Test {
  void SetUp() override {
    response1 = Http::Response();
    response2 = Http::Response();
    sstream.clear();
  }

  Http::Response response1;
  Http::Response response2;
  std::stringstream sstream;

  ResponseTest() = default;
  ResponseTest(const ResponseTest&) = delete;
  ResponseTest& operator=(const ResponseTest&) = delete;
};

TEST_F(ResponseTest, CheckStatus) {
  response1.setStatus(Http::Response::GATEWAY_TIMEOUT);
  sstream << response1;
  sstream >> response2;
  ASSERT_EQ(Http::Response::GATEWAY_TIMEOUT, response2.getStatus());
}

TEST_F(ResponseTest, CheckFields) {
  response1.setStatus(Http::Response::FORBIDDEN).addField("name1", "value1").addField("name2", "value2");
  sstream << response1;
  sstream >> response2;
  ASSERT_EQ(2, response2.getFieldCount());
  ASSERT_EQ("name1", response2.getFieldName(0));
  ASSERT_EQ("name2", response2.getFieldName(1));
  ASSERT_EQ("value1", response2.getFieldValue(0));
  ASSERT_EQ("value2", response2.getFieldValue(1));
  ASSERT_EQ("value1", response2.getFieldValue("name1"));
  ASSERT_EQ("value2", response2.getFieldValue("name2"));
  ASSERT_EQ(true, response2.hasField("name1"));
  ASSERT_EQ(false, response2.hasField("name3"));
}

TEST_F(ResponseTest, CheckString) {
  response1.setStatus(Http::Response::FORBIDDEN).addField("name1", "value1").addField("name2", "value2");
  sstream << response1;
  sstream >> response2;
  ASSERT_EQ(response1.toString(), response2.toString());
  Http::Response response3;
  response3.fromString(response1.toString());
  ASSERT_EQ(response3.toString(), response2.toString());
}
