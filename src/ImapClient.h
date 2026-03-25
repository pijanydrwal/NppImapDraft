#ifndef IMAP_CLIENT_H
#define IMAP_CLIENT_H

#include <string>
#include <vector>
#include <functional>
#include <winsock2.h>
#include <ws2tcpip.h>

struct ImapConfig {
    std::string server;
    int port = 993;
    std::string username;
    std::string password;
    std::string draftsFolder = "Drafts";
    bool useSSL = true;
};

class ImapClient {
public:
    ImapClient();
    ~ImapClient();

    bool connect(const ImapConfig& config);
    void disconnect();
    bool isConnected() const { return m_connected; }

    bool appendToDrafts(const std::string& emlContent, const std::string& folder);
    bool selectFolder(const std::string& folder);
    std::string getLastError() const { return m_lastError; }

    void setProgressCallback(std::function<void(int, int)> cb) { m_progressCb = cb; }

private:
    SOCKET m_socket;
    void* m_ssl;        // SSL*
    void* m_sslCtx;     // SSL_CTX*
    bool m_connected;
    int m_tagCounter;
    std::string m_lastError;
    std::function<void(int, int)> m_progressCb;

    bool sendCommand(const std::string& cmd, std::string& response);
    bool readResponse(std::string& response);
    bool sendData(const std::string& data);
    std::string recvLine();
    std::string nextTag();
    bool initSSL();
    void cleanupSSL();
};

#endif // IMAP_CLIENT_H
