#include "MailManager.h"
#include "../common/pushDisableWarnings.h"
#include "CSmtp_v2_2_ssl/CSmtp.h"
#include <boost/log/trivial.hpp>
#include <deque>
#include <future>
#include "../common/popDisableWarnings.h"

// "ultimate encapsulation pattern" - no includes, just declarations
std::unique_ptr<MailManager> createCSmtpMailManager(const std::string& host, uint16_t port, const std::string& user, const std::string& password);

class CSmtpMailManager : public MailManager {
public:
  CSmtpMailManager(const std::string& host, uint16_t port, const std::string& user, const std::string& password);
  ~CSmtpMailManager() override;

  // Make sure object is never copied or moved
  CSmtpMailManager(const CSmtpMailManager&) = delete;
  CSmtpMailManager& operator=(const CSmtpMailManager&) = delete;

private:
  struct Mail {
    std::string sender;
    std::string recipient;
    std::string subject;
    std::string body;
  };

  std::future<void> worker;
  std::mutex mutex;
  std::condition_variable condition;
  bool stopped;
  std::deque<Mail> mails;
  CSmtp csmtp;

  void sendMail(
    std::string&& sender,
    std::string&& recipient,
    std::string&& subject,
    std::string&& body) override;
  void runWorker();
};

CSmtpMailManager::CSmtpMailManager(const std::string& host, uint16_t port, const std::string& user, const std::string& password) : stopped(false) {
  csmtp.SetSMTPServer(host.c_str(), port);
  csmtp.SetSecurityType(USE_TLS);
  csmtp.SetLogin(user.c_str());
  csmtp.SetPassword(password.c_str());
  worker = std::async(std::launch::async, [this] { runWorker(); });
}

CSmtpMailManager::~CSmtpMailManager() {
  {
    std::lock_guard<std::mutex> lock(mutex);
    stopped = true;
  } // release mutex

  worker.wait();
}

void CSmtpMailManager::sendMail(
  std::string&& sender,
  std::string&& recipient,
  std::string&& subject,
  std::string&& body) {
  std::lock_guard<std::mutex> lock(mutex);
  mails.push_back({std::move(sender), std::move(recipient), std::move(subject), std::move(body)});
  condition.notify_one();
}

void CSmtpMailManager::runWorker() {
  Mail mail;
  for (;;) {
    {
      std::unique_lock<std::mutex> lock(mutex);
      while (!stopped && mails.empty()) {
        condition.wait(lock);
      }

      if (stopped) {
        return;
      }

      mail = mails.front();
      mails.pop_front();
    } // release mutex

    try {
      csmtp.SetSenderMail(mail.sender.c_str());
      csmtp.AddRecipient(mail.recipient.c_str());
      csmtp.SetSubject(mail.subject.c_str());
      std::size_t i = 0;
      for (std::size_t j = mail.body.find('\n', i); j != std::string::npos; j = mail.body.find('\n', i)) {
        csmtp.AddMsgLine(mail.body.substr(i, j - i).c_str());
        i = j + 1;
      }

      if (i != mail.body.size()) {
        csmtp.AddMsgLine(mail.body.substr(i).c_str());
      }

      csmtp.Send();
    } catch (...) {
      BOOST_LOG_TRIVIAL(error) << "mail sending failed";
    }

    csmtp.DelRecipients();
    csmtp.DelMsgLines();
  }
}

std::unique_ptr<MailManager> createCSmtpMailManager(const std::string& host, uint16_t port, const std::string& user, const std::string& password) {
  return std::make_unique<CSmtpMailManager>(host, port, user, password);
}
