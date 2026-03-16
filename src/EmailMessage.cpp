// EmailMessage.cpp

#include <string>

class EmailMessage {
public:
    EmailMessage(const std::string& subject, const std::string& body, const std::string& recipient)
        : subject(subject), body(body), recipient(recipient) {}

    void send() {
        // Implementation to send the email
    }

    std::string get_subject() const { return subject; }
    std::string get_body() const { return body; }
    std::string get_recipient() const { return recipient; }

private:
    std::string subject;
    std::string body;
    std::string recipient;
};

// Example usage
// EmailMessage email("Hello World", "This is a test message.", "recipient@example.com");
// email.send();