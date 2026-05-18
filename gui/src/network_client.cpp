/**
 * @file network_client.cpp
 * @brief Implementacja klienta sieciowego
 */

#include "network_client.h"
#include <cstring>
#include <chrono>
#include <thread>

#ifdef _WIN32
    static bool ws_initialized = false;
#endif

NetworkClient::NetworkClient() : socket_fd(INVALID_SOCKET_VALUE), connected(false) {
#ifdef _WIN32
    if (!ws_initialized) {
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
        ws_initialized = true;
    }
#endif
}

NetworkClient::~NetworkClient() {
    disconnect();
#ifdef _WIN32
    // Don't cleanup WSA here - other parts of app might use it
#endif
}

bool NetworkClient::connect(const std::string& ip, int port_num) {
    ip_address = ip;
    port = port_num;
    
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == INVALID_SOCKET_VALUE) {
        return false;
    }
    
#ifdef _WIN32
    u_long mode = 1;  // non-blocking
    ioctlsocket(socket_fd, FIONBIO, &mode);
#else
    // Non-blocking connect
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
#endif
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
        return false;
    }
    
    int result = ::connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
#ifdef _WIN32
    if (result < 0 && WSAGetLastError() != WSAEWOULDBLOCK) {
#else
    if (result < 0 && errno != EINPROGRESS) {
#endif
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
        return false;
    }
    
    // Wait for connection
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(socket_fd, &fds);
    
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    
    result = select(socket_fd + 1, NULL, &fds, NULL, &tv);
    if (result <= 0) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
        return false;
    }
    
    // Check for connection errors
    int so_error = 0;
    socklen_t len = sizeof(so_error);
    if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len) < 0 || so_error != 0) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
        return false;
    }
    
#ifdef _WIN32
    u_long blocking_mode = 0;
    ioctlsocket(socket_fd, FIONBIO, &blocking_mode);
#else
    // Set blocking mode
    int restore_flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, restore_flags & ~O_NONBLOCK);
#endif
    
    connected = true;
    return true;
}

void NetworkClient::disconnect() {
    if (socket_fd != INVALID_SOCKET_VALUE) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
    }
    connected = false;
}

bool NetworkClient::isConnected() const {
    return connected && socket_fd != INVALID_SOCKET_VALUE;
}

bool NetworkClient::sendTherapyPacket(const TherapyPacket& packet) {
    if (!isConnected()) return false;
    
    std::lock_guard<std::mutex> lock(tx_mutex);
    
    ssize_t sent = send(socket_fd, (const char*)&packet, sizeof(packet), 0);
    return (sent == sizeof(packet));
}

bool NetworkClient::sendCommand(const std::string& cmd) {
    if (!isConnected()) return false;
    
    std::lock_guard<std::mutex> lock(tx_mutex);
    ssize_t sent = send(socket_fd, cmd.c_str(), cmd.length(), 0);
    return (sent > 0);
}

std::string NetworkClient::receiveData(int timeout_ms) {
    if (!isConnected()) return "";
    
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(socket_fd, &fds);
    
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    
    int result = select(socket_fd + 1, &fds, NULL, NULL, &tv);
    if (result <= 0) return "";
    
    char buffer[1024];
    ssize_t received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (received > 0) {
        buffer[received] = '\0';
        return std::string(buffer, received);
    }
    
    return "";
}
