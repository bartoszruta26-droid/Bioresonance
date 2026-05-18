/**
 * @file network_client.cpp
 * @brief Implementacja klienta sieciowego
 */

#include "network_client.h"
#include "error_handler.h"
#include <cstring>
#include <chrono>
#include <thread>
#include <atomic>

#ifdef _WIN32
    // Reference counter for WSA initialization with proper cleanup
    static std::atomic<int> ws_init_count{0};
#else
    #include <cerrno>
#endif

NetworkClient::NetworkClient() : socket_fd(INVALID_SOCKET_VALUE), connected(false) {
#ifdef _WIN32
    if (ws_init_count++ == 0) {
        WSADATA wsa_data;
        int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (result != 0) {
            REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                        "WSAStartup failed: " + std::to_string(result));
        }
    }
#endif
}

NetworkClient::~NetworkClient() {
    disconnect();
#ifdef _WIN32
    if (--ws_init_count == 0) {
        WSACleanup();
    }
#endif
}

bool NetworkClient::connect(const std::string& ip, int port_num) {
    // Validate IP address format (basic check)
    if (ip.empty()) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_UNREACHABLE, ErrorCategory::NETWORK,
                    "Empty IP address");
        return false;
    }
    
    // Validate port range
    if (port_num < 1 || port_num > 65535) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                    "Invalid port number: " + std::to_string(port_num));
        return false;
    }
    
    ip_address = ip;
    port = port_num;
    
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == INVALID_SOCKET_VALUE) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                    "Failed to create socket");
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
        REPORT_ERROR(ErrorCode::ERR_NETWORK_UNREACHABLE, ErrorCategory::NETWORK,
                    "Invalid IP address format: " + ip);
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
        REPORT_ERROR(ErrorCode::ERR_NETWORK_UNREACHABLE, ErrorCategory::NETWORK,
                    "Connection failed");
        return false;
    }
    
    // Wait for connection - use portable select with proper socket handling
    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(socket_fd, &write_fds);
    
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    
#ifdef _WIN32
    result = select(0, NULL, &write_fds, NULL, &tv);
#else
    result = select(socket_fd + 1, NULL, &write_fds, NULL, &tv);
#endif
    if (result <= 0) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
        REPORT_ERROR(ErrorCode::ERR_NETWORK_TIMEOUT, ErrorCategory::NETWORK,
                    "Connection timeout");
        return false;
    }
    
    // Check for connection errors - portable cast for getsockopt
    int so_error = 0;
    socklen_t len = sizeof(so_error);
#ifdef _WIN32
    if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len) < 0 || so_error != 0) {
#else
    if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0 || so_error != 0) {
#endif
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
        REPORT_ERROR(ErrorCode::ERR_NETWORK_UNREACHABLE, ErrorCategory::NETWORK,
                    "Connection error: " + std::to_string(so_error));
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
    LOG_INFO("Connected to " + ip + ":" + std::to_string(port));
    return true;
}

void NetworkClient::disconnect() {
    if (socket_fd != INVALID_SOCKET_VALUE) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
    }
    connected = false;
    LOG_INFO("Disconnected from device");
}

bool NetworkClient::isConnected() const {
    return connected && socket_fd != INVALID_SOCKET_VALUE;
}

bool NetworkClient::sendTherapyPacket(const TherapyPacket& packet) {
    if (!isConnected()) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_DISCONNECTED, ErrorCategory::NETWORK, 
                    "Cannot send therapy packet - not connected");
        return false;
    }
    
    std::lock_guard<std::mutex> lock(tx_mutex);
    
    ssize_t sent = send(socket_fd, (const char*)&packet, sizeof(packet), 0);
    if (sent < 0) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SEND_FAILED, ErrorCategory::NETWORK,
                    "Failed to send therapy packet");
        return false;
    }
    return (static_cast<size_t>(sent) == sizeof(packet));
}

bool NetworkClient::sendCommand(const std::string& cmd) {
    if (!isConnected()) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_DISCONNECTED, ErrorCategory::NETWORK,
                    "Cannot send command - not connected");
        return false;
    }
    
    std::lock_guard<std::mutex> lock(tx_mutex);
    ssize_t sent = send(socket_fd, cmd.c_str(), cmd.length(), 0);
    if (sent < 0) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SEND_FAILED, ErrorCategory::NETWORK,
                    "Failed to send command");
        return false;
    }
    return (sent > 0);
}

std::string NetworkClient::receiveData(int timeout_ms) {
    if (!isConnected()) return "";
    
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(socket_fd, &read_fds);
    
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    
#ifdef _WIN32
    int result = select(0, &read_fds, NULL, NULL, &tv);
#else
    int result = select(socket_fd + 1, &read_fds, NULL, NULL, &tv);
#endif
    if (result <= 0) return "";
    
    char buffer[1024];
    ssize_t received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (received < 0) {
#ifdef _WIN32
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            REPORT_ERROR(ErrorCode::ERR_NETWORK_RECV_FAILED, ErrorCategory::NETWORK,
                        "Failed to receive data");
        }
#else
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            REPORT_ERROR(ErrorCode::ERR_NETWORK_RECV_FAILED, ErrorCategory::NETWORK,
                        "Failed to receive data");
        }
#endif
        return "";
    }
    if (received > 0) {
        buffer[received] = '\0';
        return std::string(buffer, static_cast<size_t>(received));
    }
    
    return "";
}
