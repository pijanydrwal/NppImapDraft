#include "EmailMessage.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>

std::string EmailMessage::formatDate(time_t t) {
    struct tm* tm_info = gmtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S +0000", tm_info);
    return std::string(buf);
}

std::string EmailMessage::generateMsgId() {
    char buf[64];
    snprintf(buf, sizeof(buf), "<%lld.npp@imapplug>", (long long)time(nullptr));
    return std::string(buf);
}

std::string EmailMessage::encodeBase64(const std::string& data) {
    static const char* b64 =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(b64[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

std::string EmailMessage::toEML() const {
    std::ostringstream ss;
    ss << "MIME-Version: 1.0\r\n";
    ss << "Date: " << formatDate(date) << "\r\n";
    ss << "Message-ID: " << generateMsgId() << "\r\n";
    ss << "From: " << (from.empty() ? "npp-plugin@localhost" : from) << "\r\n";
    ss << "To: " << (to.empty() ? "draft@localhost" : to) << "\r\n";
    ss << "Subject: " << subject << "\r\n";

    if (attachments.empty() && htmlBody.empty()) {
        ss << "Content-Type: text/plain; charset=UTF-8\r\n";
        ss << "Content-Transfer-Encoding: 8bit\r\n";
        ss << "\r\n";
        ss << body;
    } else {
        std::string boundary = "----=_NppBnd_" + std::to_string(date);
        if (!htmlBody.empty()) {
            ss << "Content-Type: multipart/alternative; boundary=\"" << boundary << "\"\r\n";
            ss << "\r\n";
            ss << "--" << boundary << "\r\n";
            ss << "Content-Type: text/plain; charset=UTF-8\r\n\r\n";
            ss << body << "\r\n";
            ss << "--" << boundary << "\r\n";
            ss << "Content-Type: text/html; charset=UTF-8\r\n\r\n";
            ss << htmlBody << "\r\n";
            ss << "--" << boundary << "--\r\n";
        } else {
            ss << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"\r\n";
            ss << "\r\n";
            ss << "--" << boundary << "\r\n";
            ss << "Content-Type: text/plain; charset=UTF-8\r\n\r\n";
            ss << body << "\r\n";
            for (const auto& att : attachments) {
                ss << "--" << boundary << "\r\n";
                ss << "Content-Type: " << att.mimeType << "; name=\"" << att.filename << "\"\r\n";
                ss << "Content-Transfer-Encoding: base64\r\n";
                ss << "Content-Disposition: attachment; filename=\"" << att.filename << "\"\r\n\r\n";
                ss << att.content << "\r\n";
            }
            ss << "--" << boundary << "--\r\n";
        }
    }
    return ss.str();
}

EmailMessage EmailMessage::fromBuffer(const std::string& filename,
                                      const std::string& content) {
    EmailMessage msg;
    msg.subject = filename.empty() ? "Untitled Draft" : filename;
    msg.body = content;
    msg.date = time(nullptr);
    // Auto-detect if content looks like HTML
    if (content.find("<html") != std::string::npos ||
        content.find("<HTML") != std::string::npos) {
        msg.htmlBody = content;
        msg.body = "[HTML content - see HTML part]";
    }
    return msg;
}
