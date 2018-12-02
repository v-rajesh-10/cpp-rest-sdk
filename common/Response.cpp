#include "Response.h"
#include "pushDisableWarnings.h"
#include <sstream>
#include "popDisableWarnings.h"

namespace Http {

namespace {

struct StatusInfo {
  Response::Status code;
  const char* text;
};

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

StatusInfo statuses[]{
  {Response::CONTINUE, "Continue"},
  {Response::SWITCHING_PROTOCOLS, "Switching Protocols"},
  {Response::OK, "OK"},
  {Response::CREATED, "Created"},
  {Response::ACCEPTED, "Accepted"},
  {Response::NON_AUTHORITATIVE_INFORMATION, "Non-Authoritative Information"},
  {Response::NO_CONTENT, "No Content"},
  {Response::RESET_CONTENT, "Reset Content"},
  {Response::PARTIAL_CONTENT, "Partial Content"},
  {Response::MULTIPLE_CHOICES, "Multiple Choices"},
  {Response::MOVED_PERMANENTLY, "Moved Permanently"},
  {Response::FOUND, "Found"},
  {Response::SEE_OTHER, "See Other"},
  {Response::NOT_MODIFIED, "Not Modified"},
  {Response::USE_PROXY, "Use Proxy"},
  {Response::TEMPORARY_REDIRECT, "Temporary Redirect"},
  {Response::BAD_REQUEST, "Bad Request"},
  {Response::UNAUTHORIZED, "Unauthorized"},
  {Response::PAYMENT_REQUIRED, "Payment Required"},
  {Response::FORBIDDEN, "Forbidden"},
  {Response::NOT_FOUND, "Not Found"},
  {Response::METHOD_NOT_ALLOWED, "Method Not Allowed"},
  {Response::NOT_ACCEPTABLE, "Not Acceptable"},
  {Response::PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required"},
  {Response::REQUEST_TIMEOUT, "Request Timeout"},
  {Response::CONFLICT, "Conflict"},
  {Response::GONE, "Gone"},
  {Response::LENGTH_REQUIRED, "Length Required"},
  {Response::PRECONDITION_FAILED, "Precondition Failed"},
  {Response::REQUEST_ENTITY_TOO_LARGE, "Request Entity Too Large"},
  {Response::REQUEST_URI_TOO_LONG, "Request-URI Too Long"},
  {Response::UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
  {Response::REQUESTED_RANGE_NOT_SATISFIABLE, "Requested Range Not Satisfiable"},
  {Response::EXPECTATION_FAILED, "Expectation Failed"},
  {Response::INTERNAL_SERVER_ERROR, "Internal Server Error"},
  {Response::NOT_IMPLEMENTED, "Not Implemented"},
  {Response::BAD_GATEWAY, "Bad Gateway"},
  {Response::SERVICE_UNAVAILABLE, "Service Unavailable"},
  {Response::GATEWAY_TIMEOUT, "Gateway Timeout"},
  {Response::HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"}
};

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

Response::Response() : status(OK) {}

Response::Status Response::getStatus() const {
  return status;
}

Response& Response::setStatus(Status status0) {
  status = status0;
  return *this;
}

std::size_t Response::getFieldCount() const {
  return fields.size();
}

std::string Response::getFieldName(std::size_t index) const {
  return fields[index].name;
}

std::string Response::getFieldValue(std::size_t index) const {
  return fields[index].value;
}

bool Response::hasField(const std::string& name) const {
  std::string upperCaseName = toUpperCase(name);
  for (const Field& field : fields) {
    if (toUpperCase(field.name) == upperCaseName) {
      return true;
    }
  }

  return false;
}

std::string Response::getFieldValue(const std::string& name) const {
  std::string upperCaseName = toUpperCase(name);
  for (const Field& field : fields) {
    if (toUpperCase(field.name) == upperCaseName) {
      return field.value;
    }
  }

  return "";
}

Response& Response::addField(const std::string& name, const std::string& value) {
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

const std::vector<uint8_t>& Response::getBody() const {
  return body;
}

Response& Response::setBody(const std::vector<uint8_t>& body0) {
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

void Response::fromString(const std::string& string) {
  std::istringstream(string) >> *this;
}

std::string Response::toString() const {
  std::ostringstream stream;
  stream << *this;
  return stream.str();
}

std::istream& operator>>(std::istream& in, Response& response) {
  std::string firstLine = extract(in, "\r\n");
  std::size_t spacePosition = firstLine.find(' ');
  if (spacePosition == std::string::npos) {
    throw std::runtime_error("Invalid first line \"" + firstLine + '"');
  }

  if (firstLine.substr(0, spacePosition) != "HTTP/1.1") {
    throw std::runtime_error("Unknown protocol \"" + firstLine.substr(0, spacePosition) + '"');
  }

  firstLine = firstLine.substr(spacePosition + 1);
  spacePosition = firstLine.find(' ');
  uint32_t statusCode;
  std::string statusText = "";
  if (spacePosition != std::string::npos) {
    std::istringstream(firstLine.substr(0, spacePosition)) >> statusCode;
    statusText = firstLine.substr(spacePosition + 1);
  } else {
    std::istringstream(firstLine) >> statusCode;
  }

  bool found = false;
  for (std::size_t i = 0; i < sizeof(statuses) / sizeof(statuses[0]); ++i) {
    if (statuses[i].code == static_cast<Response::Status>(statusCode)) {
      if (statuses[i].text != statusText) {
        throw std::runtime_error(std::string("Inconsistent HTTP status text \"") + statusText + '"');
      }

      response.status = statuses[i].code;
      found = true;
      break;
    }
  }

  if (!found) {
    throw std::runtime_error(std::string("Unknown HTTP status code ") + std::to_string(statusCode) + ' ' + statusText);
  }

  response.fields.clear();
  response.body.clear();
  for (;;) {
    std::string field = extract(in, "\r\n");
    if (field.empty()) {
      break;
    }

    std::size_t colonPosition = field.find(':');
    if (colonPosition == std::string::npos || colonPosition + 1 == field.size() || field[colonPosition + 1] != ' ') {
      throw std::runtime_error("Invalid header field format");
    }

    response.fields.emplace_back(Response::Field{field.substr(0, colonPosition), field.substr(colonPosition + 2)});
  }

  if (response.hasField("Content-Length")) {
    std::size_t contentSize;
    std::istringstream(response.getFieldValue("Content-Length")) >> contentSize;
    response.body.resize(contentSize);
    if (contentSize != 0) {
      in.read(reinterpret_cast<char*>(response.body.data()), std::streamsize(contentSize));
      if (!in.good()) {
        throw std::runtime_error("Unable to read from input stream");
      }
    }
  } else if (response.hasField("Transfer-Encoding")) {
    std::string transferEncoding = response.getFieldValue("Transfer-Encoding");
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

      response.body.resize(offset + chunkSize);
      in.read(reinterpret_cast<char*>(response.body.data() + offset), std::streamsize(chunkSize));
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

std::ostream& operator<<(std::ostream& out, const Response& response) {
  out << "HTTP/1.1 " << response.status << ' ';
  for (std::size_t i = 0; i < sizeof(statuses) / sizeof(statuses[0]); ++i) {
    if (statuses[i].code == response.status) {
      out << statuses[i].text << "\r\n";
      break;
    }
  }

  for (const Response::Field& field : response.fields) {
    out << field.name << ": " << field.value << "\r\n";
  }

  out << "\r\n";
  if (response.hasField("Content-Length")) {
    out.write(reinterpret_cast<const char*>(response.body.data()), std::streamsize(response.body.size()));
  } else if (response.getFieldValue("Transfer-Encoding") == "chunked") {
    out << std::hex << response.body.size() << std::dec << "\r\n";
    out.write(reinterpret_cast<const char*>(response.body.data()), std::streamsize(response.body.size()));
    out << "\r\n0\r\n\r\n";
  }

  return out;
}

}
