#include "Request.h"
#include "url.h"
#include "pushDisableWarnings.h"
#include <regex>
#include <sstream>
#include "popDisableWarnings.h"

namespace Http {

namespace {

const char characterUpperCases[256] = {
  '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\x09', '\x0A', '\x0B', '\x0C', '\x0D', '\x0E', '\x0F',
  '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F',
  '\x20', '\x21', '\x22', '\x23', '\x24', '\x25', '\x26', '\x27', '\x28', '\x29', '\x2A', '\x2B', '\x2C', '\x2D', '\x2E', '\x2F',
  '\x30', '\x31', '\x32', '\x33', '\x34', '\x35', '\x36', '\x37', '\x38', '\x39', '\x3A', '\x3B', '\x3C', '\x3D', '\x3E', '\x3F',
  '\x40', '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47', '\x48', '\x49', '\x4A', '\x4B', '\x4C', '\x4D', '\x4E', '\x4F',
  '\x50', '\x51', '\x52', '\x53', '\x54', '\x55', '\x56', '\x57', '\x58', '\x59', '\x5A', '\x5B', '\x5C', '\x5D', '\x5E', '\x5F',
  '\x60', '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47', '\x48', '\x49', '\x4A', '\x4B', '\x4C', '\x4D', '\x4E', '\x4F',
  '\x50', '\x51', '\x52', '\x53', '\x54', '\x55', '\x56', '\x57', '\x58', '\x59', '\x5A', '\x7B', '\x7C', '\x7D', '\x7E', '\x7F',
  '\x80', '\x81', '\x82', '\x83', '\x84', '\x85', '\x86', '\x87', '\x88', '\x89', '\x8A', '\x8B', '\x8C', '\x8D', '\x8E', '\x8F',
  '\x90', '\x91', '\x92', '\x93', '\x94', '\x95', '\x96', '\x97', '\x98', '\x99', '\x9A', '\x9B', '\x9C', '\x9D', '\x9E', '\x9F',
  '\xA0', '\xA1', '\xA2', '\xA3', '\xA4', '\xA5', '\xA6', '\xA7', '\xA8', '\xA9', '\xAA', '\xAB', '\xAC', '\xAD', '\xAE', '\xAF',
  '\xB0', '\xB1', '\xB2', '\xB3', '\xB4', '\xB5', '\xB6', '\xB7', '\xB8', '\xB9', '\xBA', '\xBB', '\xBC', '\xBD', '\xBE', '\xBF',
  '\xC0', '\xC1', '\xC2', '\xC3', '\xC4', '\xC5', '\xC6', '\xC7', '\xC8', '\xC9', '\xCA', '\xCB', '\xCC', '\xCD', '\xCE', '\xCF',
  '\xD0', '\xD1', '\xD2', '\xD3', '\xD4', '\xD5', '\xD6', '\xD7', '\xD8', '\xD9', '\xDA', '\xDB', '\xDC', '\xDD', '\xDE', '\xDF',
  '\xE0', '\xE1', '\xE2', '\xE3', '\xE4', '\xE5', '\xE6', '\xE7', '\xE8', '\xE9', '\xEA', '\xEB', '\xEC', '\xED', '\xEE', '\xEF',
  '\xF0', '\xF1', '\xF2', '\xF3', '\xF4', '\xF5', '\xF6', '\xF7', '\xF8', '\xF9', '\xFA', '\xFB', '\xFC', '\xFD', '\xFE', '\xFF'
};

const char* methodStrings[]{"GET", "POST"};

bool endsWithUnsafe(const std::string& string1, const std::string& string2) {
  return std::memcmp(string1.data() + string1.size() - string2.size(), string2.data(), string2.size()) == 0;
}

std::string toUpperCase(const std::string& source) {
  std::string result = source;
  for (char& c : result) {
    c = characterUpperCases[static_cast<unsigned char>(c)];
  }

  return result;
}

std::string extract(std::istream& in, char delimiter) {
  std::string result;
  for (;;) {
    char c;
    in.get(c);
    if (!in.good()) {
      throw std::runtime_error("Unable to read from input stream");
    }

    if (c == delimiter) {
      break;
    }

    result += c;
  }

  return result;
}

std::string extract(std::istream& in, const std::string& delimiter) {
  std::string result;
  for (std::size_t i = 0; i < delimiter.size(); ++i) {
    char c;
    in.get(c);
    if (!in.good()) {
      throw std::runtime_error("Unable to read from input stream");
    }

    result += c;
  }

  while (!endsWithUnsafe(result, delimiter)) {
    char c;
    in.get(c);
    if (!in.good()) {
      throw std::runtime_error("Unable to read from input stream");
    }

    result += c;
  }

  result.resize(result.size() - delimiter.size());
  return result;
}

}

Request::Request() : method(GET) {}

Request::Method Request::getMethod() const {
  return method;
}

Request& Request::setMethod(Method method0) {
  method = method0;
  return *this;
}

std::string Request::getPath() const {
  return path;
}

Request& Request::setPath(const std::string& path0) {
  path = path0;
  return *this;
}

std::size_t Request::getPathParameterCount() const {
  return pathParameters.size();
}

std::string Request::getPathParameterName(std::size_t index) const {
  return pathParameters[index].name;
}

std::string Request::getPathParameterValue(std::size_t index) const {
  return pathParameters[index].value;
}

bool Request::hasPathParameter(const std::string& name) const {
  for (const PathParameter& parameter : pathParameters) {
    if (parameter.name == name) {
      return true;
    }
  }

  return false;
}

std::string Request::getPathParameterValue(const std::string& name) const {
  for (const PathParameter& parameter : pathParameters) {
    if (parameter.name == name) {
      return parameter.value;
    }
  }

  return "";
}

Request& Request::addPathParameter(const std::string& name, const std::string& value) {
  pathParameters.push_back({name, value});
  return *this;
}

std::size_t Request::getFieldCount() const {
  return fields.size();
}

std::string Request::getFieldName(std::size_t index) const {
  return fields[index].name;
}

std::string Request::getFieldValue(std::size_t index) const {
  return fields[index].value;
}

bool Request::hasField(const std::string& name) const {
  std::string upperCaseName = toUpperCase(name);
  for (const Field& field : fields) {
    if (toUpperCase(field.name) == upperCaseName) {
      return true;
    }
  }

  return false;
}

std::string Request::getFieldValue(const std::string& name) const {
  std::string upperCaseName = toUpperCase(name);
  for (const Field& field : fields) {
    if (toUpperCase(field.name) == upperCaseName) {
      return field.value;
    }
  }

  return "";
}

Request& Request::addField(const std::string& name, const std::string& value) {
  std::string upperCaseName = toUpperCase(name);
  for (const Field& field : fields) {
    if (toUpperCase(field.name) == upperCaseName) {
      throw std::runtime_error("Field already exists");
    }
  }

  if (upperCaseName == "CONTENT-LENGTH") {
    if (value != "0") {
      throw std::runtime_error("Initial value for Content-Length is not 0");
    }

    if (getFieldValue("Transfer-Encoding") == "chunked") {
      throw std::runtime_error("Transfer-Encoding already set to chunked");
    }
  } else if (upperCaseName == "TRANSFER-ENCODING") {
    if (value != "chunked") {
      throw std::runtime_error("Unsupported value for Transfer-Encoding");
    }

    if (hasField("Content-Length")) {
      throw std::runtime_error("Content-Length already set");
    }
  }

  fields.emplace_back(Field{name, value});
  return *this;
}

const std::vector<uint8_t>& Request::getBody() const {
  return body;
}

Request& Request::setBody(std::vector<uint8_t>&& body0) {
  for (Field& field : fields) {
    std::string upperCaseName = toUpperCase(field.name);
    if (upperCaseName == "CONTENT-LENGTH") {
      field.value = std::to_string(body0.size());
      body = std::move(body0);
      return *this;
    }

    if (upperCaseName == "TRANSFER-ENCODING") {
      body = std::move(body0);
      return *this;
    }
  }

  fields.emplace_back(Field{"Transfer-Encoding", "chunked"});
  body = std::move(body0);
  return *this;
}

void Request::fromString(const std::string& string) {
  std::istringstream(string) >> *this;
}

std::string Request::toString() const {
  std::ostringstream stream;
  stream << *this;
  return stream.str();
}

std::istream& operator>>(std::istream& in, Request& request) {
  std::string methodString = extract(in, ' ');
  bool found = false;
  for (std::size_t i = 0; i < sizeof(methodStrings) / sizeof(methodStrings[0]); ++i) {
    if (methodStrings[i] == methodString) {
      request.method = static_cast<Request::Method>(i);
      found = true;
      break;
    }
  }

  if (!found) {
    throw std::runtime_error("Unknown HTTP method \"" + methodString + '"');
  }

  std::string path = extract(in, ' ');

  request.pathParameters.clear();
  auto qmarkPos = path.find_first_of('?');
  if (qmarkPos != std::string::npos) {
    std::string parametersString = path.substr(qmarkPos);
    std::regex pattern("([\\w+%]+)=([^&]*)");
    for (std::sregex_iterator i = std::sregex_iterator(parametersString.begin(), parametersString.end(), pattern); i != std::sregex_iterator(); i++) {
      request.pathParameters.push_back({urlDecode((*i)[1].str()), urlDecode((*i)[2].str())});
    }
  }

  request.path = path.substr(0, qmarkPos);
  std::string protocol = extract(in, "\r\n");
  if (protocol != "HTTP/1.1") {
    throw std::runtime_error("Unknown protocol \"" + protocol + '"');
  }

  request.fields.clear();
  request.body.clear();
  for (;;) {
    std::string field = extract(in, "\r\n");
    if (field.empty()) {
      break;
    }

    std::size_t colonPosition = field.find(':');
    if (colonPosition == std::string::npos || colonPosition + 1 == field.size() || field[colonPosition + 1] != ' ') {
      throw std::runtime_error("Invalid header field format");
    }

    request.fields.emplace_back(Request::Field{field.substr(0, colonPosition), field.substr(colonPosition + 2)});
  }

  if (request.hasField("Content-Length")) {
    std::size_t contentSize;
    std::istringstream(request.getFieldValue("Content-Length")) >> contentSize;
    request.body.resize(contentSize);
    if (contentSize != 0) {
      in.read(reinterpret_cast<char*>(request.body.data()), std::streamsize(contentSize));
      if (!in.good()) {
        throw std::runtime_error("Unable to read from input stream");
      }
    }
  } else if (request.hasField("Transfer-Encoding")) {
    std::string transferEncoding = request.getFieldValue("Transfer-Encoding");
    if (transferEncoding != "chunked") {
      throw std::runtime_error("Unsupported value for Transfer-Encoding");
    }

    std::size_t offset = 0;
    for (;;) {
      std::size_t chunkSize;
      std::istringstream(extract(in, "\r\n")) >> std::hex >> chunkSize;
      if (chunkSize == 0) {
        char c;
        in.get(c);
        if (!in.good() || c != '\r') {
          throw std::runtime_error("Unable to read from input stream");
        }

        in.get(c);
        if (!in.good() || c != '\n') {
          throw std::runtime_error("Unable to read from input stream");
        }

        break;
      }

      request.body.resize(offset + chunkSize);
      in.read(reinterpret_cast<char*>(request.body.data() + offset), std::streamsize(chunkSize));
      offset += chunkSize;
      if (!in.good()) {
        throw std::runtime_error("Unable to read from input stream");
      }

      char c;
      in.get(c);
      if (!in.good() || c != '\r') {
        throw std::runtime_error("Unable to read from input stream");
      }

      in.get(c);
      if (!in.good() || c != '\n') {
        throw std::runtime_error("Unable to read from input stream");
      }
    }
  }

  return in;
}

std::ostream& operator<<(std::ostream& out, const Request& request) {
  std::string parameters;
  char delimeter = '?';
  for (auto& parameter : request.pathParameters) {
    parameters += delimeter + urlEncode(parameter.name) + '=' + urlEncode(parameter.value);
    delimeter = '&';
  }

  out << methodStrings[request.method] << ' ' << request.path << parameters << ' ' << "HTTP/1.1\r\n";
  for (const Request::Field& field : request.fields) {
    out << field.name << ": " << field.value << "\r\n";
  }

  out << "\r\n";
  if (request.hasField("Content-Length")) {
    out.write(reinterpret_cast<const char*>(request.body.data()), std::streamsize(request.body.size()));
  } else if (request.getFieldValue("Transfer-Encoding") == "chunked") {
    out << std::hex << request.body.size() << std::dec << "\r\n";
    out.write(reinterpret_cast<const char*>(request.body.data()), std::streamsize(request.body.size()));
    out << "\r\n0\r\n\r\n";
  }

  return out;
}

std::string Request::getMethodString(Method method) {
  return methodStrings[method];
}

}
