#pragma once

#include "../common/pushDisableWarnings.h"
#include <string>
#include "../common/popDisableWarnings.h"

class MailManager {
public:
  virtual ~MailManager() = default;
  virtual void sendMail(
    std::string&& sender,
    std::string&& recipient,
    std::string&& subject,
    std::string&& body) = 0;
};
