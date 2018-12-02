#pragma once

#include "pushDisableWarnings.h"
#include <vector>
#include "popDisableWarnings.h"

namespace Http {

class Request {
public:
  enum Method {
    GET = 0,
    POST = 1
  };

  Request();
  Method getMethod() const;
  Request& setMethod(Method method);
  std::string getPath() const;
  Request& setPath(const std::string& path);
  std::size_t getPathParameterCount() const;
  std::string getPathParameterName(std::size_t index) const;
  std::string getPathParameterValue(std::size_t index) const;
  bool hasPathParameter(const std::string& name) const;
  std::string getPathParameterValue(const std::string& name) const;
  Request& addPathParameter(const std::string& name, const std::string& value);
  std::size_t getFieldCount() const;
  std::string getFieldName(std::size_t index) const;
  std::string getFieldValue(std::size_t index) const;
  bool hasField(const std::string& name) const;
  std::string getFieldValue(const std::string& name) const;
  Request& addField(const std::string& name, const std::string& value);
  const std::vector<uint8_t>& getBody() const;
  Request& setBody(std::vector<uint8_t>&& body);
  void fromString(const std::string& string);
  std::string toString() const;
  friend std::istream& operator>>(std::istream& in, Request& request);
  friend std::ostream& operator<<(std::ostream& out, const Request& request);
  static std::string getMethodString(Method method);

private:
  struct Field {
    std::string name;
    std::string value;
  };

  struct PathParameter {
    std::string name;
    std::string value;
  };

  Method method;
  std::string path;
  std::vector<Field> fields;
  std::vector<PathParameter> pathParameters;
  std::vector<uint8_t> body;
};

}
