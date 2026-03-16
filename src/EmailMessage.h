#ifndef EMAILMESSAGE_H
#define EMAILMESSAGE_H

#include <string>

class EmailMessage {
public:
    std::string to;
    std::string from;
    std::string subject;
    std::string date;
    std::string body;

    EmailMessage(const std::string& to, const std::string& from, const std::string& subject, const std::string& date, const std::string& body)
        : to(to), from(from), subject(subject), date(date), body(body) {}
};

#endif // EMAILMESSAGE_H
