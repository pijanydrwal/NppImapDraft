#ifndef EMAIL_MESSAGE_H
#define EMAIL_MESSAGE_H

#include <string>
#include <vector>
#include <ctime>

struct EmailAttachment {
    std::string filename;
    std::string content;  // base64 encoded
    std::string mimeType;
};

class EmailMessage {
public:
    std::string subject;
    std::string from;
    std::string to;
    std::string body;          // plain text body
    std::string htmlBody;      // optional HTML body
    std::vector<EmailAttachment> attachments;
    time_t date = 0;

    EmailMessage() { date = time(nullptr); }

    // Build RFC 2822 / EML string suitable for IMAP APPEND
    std::string toEML() const;

    // Create from Notepad++ buffer
    static EmailMessage fromBuffer(const std::string& filename,
                                   const std::string& content);

private:
    static std::string formatDate(time_t t);
    static std::string encodeBase64(const std::string& data);
    static std::string generateMsgId();
};

#endif // EMAIL_MESSAGE_H
