#include "ImapClient.h"
#include <sstream>
#include <stdexcept>
#pragma comment(lib, "ws2_32.lib")

// We use OpenSSL via dynamic linking
// If OpenSSL not available, compile with IMAP_NO_SSL for plain-text fallback
#ifndef IMAP_NO_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")
#endif

ImapClient::ImapClient()
    : m_socket(INVALID_SOCKET), m_ssl(nullptr), m_sslCtx(nullptr),
      m_connected(false), m_tagCounter(0) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
}

ImapClient::~ImapClient() {
    disconnect();
    WSACleanup();
}

bool ImapClient::initSSL() {
#ifndef IMAP_NO_SSL
    SSL_library_init();
    SSL_load_error_strings();
    const SSL_METHOD* method = TLS_client_method();
    m_sslCtx = SSL_CTX_new(method);
    if (!m_sslCtx) { m_lastError = "SSL_CTX_new failed"; return false; }
    m_ssl = SSL_new((SSL_CTX*)m_sslCtx);
    if (!m_ssl) { m_lastError = "SSL_new failed"; return false; }
    SSL_set_fd((SSL*)m_ssl, (int)m_socket);
    if (SSL_connect((SSL*)m_ssl) <= 0) {
        m_lastError = "SSL_connect failed";
        return false;
    }
    return true;
#else
    return true;
#endif
}

void ImapClient::cleanupSSL() {
#ifndef IMAP_NO_SSL
    if (m_ssl) { SSL_shutdown((SSL*)m_ssl); SSL_free((SSL*)m_ssl); m_ssl = nullptr; }
    if (m_sslCtx) { SSL_CTX_free((SSL_CTX*)m_sslCtx); m_sslCtx = nullptr; }
#endif
}

bool ImapClient::connect(const ImapConfig& config) {
    struct addrinfo hints = {}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    std::string portStr = std::to_string(config.port);
    if (getaddrinfo(config.server.c_str(), portStr.c_str(), &hints, &res) != 0) {
        m_lastError = "DNS resolution failed: " + config.server;
        return false;
    }
    m_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (m_socket == INVALID_SOCKET) { freeaddrinfo(res); m_lastError = "socket() failed"; return false; }
    if (::connect(m_socket, res->ai_addr, (int)res->ai_addrlen) != 0) {
        freeaddrinfo(res); m_lastError = "connect() failed"; return false;
    }
    freeaddrinfo(res);
    if (config.useSSL && !initSSL()) return false;
    // Read server greeting
    std::string greeting;
    recvLine();
    // LOGIN
    std::string resp;
    std::string loginCmd = "LOGIN " + config.username + " " + config.password;
    if (!sendCommand(loginCmd, resp)) return false;
    if (resp.find("OK") == std::string::npos) {
        m_lastError = "LOGIN failed: " + resp;
        return false;
    }
    m_connected = true;
    return true;
}

void ImapClient::disconnect() {
    if (m_connected) {
        std::string resp;
        sendCommand("LOGOUT", resp);
        m_connected = false;
    }
    cleanupSSL();
    if (m_socket != INVALID_SOCKET) { closesocket(m_socket); m_socket = INVALID_SOCKET; }
}

std::string ImapClient::nextTag() {
    return "A" + std::to_string(++m_tagCounter);
}

bool ImapClient::sendData(const std::string& data) {
#ifndef IMAP_NO_SSL
    if (m_ssl) {
        int r = SSL_write((SSL*)m_ssl, data.c_str(), (int)data.size());
        return r > 0;
    }
#endif
    int r = send(m_socket, data.c_str(), (int)data.size(), 0);
    return r != SOCKET_ERROR;
}

std::string ImapClient::recvLine() {
    std::string line;
    char ch;
    while (true) {
#ifndef IMAP_NO_SSL
        int r = m_ssl ? SSL_read((SSL*)m_ssl, &ch, 1) : recv(m_socket, &ch, 1, 0);
#else
        int r = recv(m_socket, &ch, 1, 0);
#endif
        if (r <= 0) break;
        if (ch == '\n') break;
        if (ch != '\r') line += ch;
    }
    return line;
}

bool ImapClient::sendCommand(const std::string& cmd, std::string& response) {
    std::string tag = nextTag();
    std::string fullCmd = tag + " " + cmd + "\r\n";
    if (!sendData(fullCmd)) { m_lastError = "send failed"; return false; }
    response.clear();
    while (true) {
        std::string line = recvLine();
        response += line + "\n";
        if (line.substr(0, tag.size()) == tag) break;
        if (line.empty()) break;
    }
    return true;
}

bool ImapClient::selectFolder(const std::string& folder) {
    std::string resp;
    if (!sendCommand("SELECT \"" + folder + "\"", resp)) return false;
    return resp.find("OK") != std::string::npos;
}

bool ImapClient::appendToDrafts(const std::string& emlContent, const std::string& folder) {
    std::string tag = nextTag();
    std::string cmd = tag + " APPEND \"" + folder + "\" (\\Draft \\Seen) {" +
                      std::to_string(emlContent.size()) + "}\r\n";
    if (!sendData(cmd)) { m_lastError = "APPEND cmd send failed"; return false; }
    // Wait for continuation
    std::string cont = recvLine();
    if (cont.empty() || cont[0] != '+') {
        m_lastError = "Server did not send continuation: " + cont;
        return false;
    }
    // Send EML data
    if (!sendData(emlContent + "\r\n")) { m_lastError = "EML data send failed"; return false; }
    // Read tagged response
    std::string resp;
    while (true) {
        std::string line = recvLine();
        resp += line + "\n";
        if (line.substr(0, tag.size()) == tag) break;
        if (line.empty()) break;
    }
    if (resp.find("OK") == std::string::npos) {
        m_lastError = "APPEND failed: " + resp;
        return false;
    }
    return true;
}
