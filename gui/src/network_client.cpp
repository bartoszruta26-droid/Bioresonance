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
    // Serialize Winsock lifetime management; count only successful acquisitions.
    static std::mutex ws_init_mutex;
    static int ws_init_count = 0;
    static bool ws_init_ok = false;
#else
    #include <cerrno>
#endif

NetworkClient::NetworkClient()
    : socket_fd(INVALID_SOCKET_VALUE), connected(false)
#ifdef _WIN32
    , winsock_acquired(false)
#endif
{
#ifdef _WIN32
    int startup_error = 0;
    {
        std::lock_guard<std::mutex> lock(ws_init_mutex);
        if (ws_init_count == 0) {
            WSADATA wsa_data;
            startup_error = WSAStartup(MAKEWORD(2, 2), &wsa_data);
            ws_init_ok = (startup_error == 0);
        }
        if (ws_init_ok) {
            ++ws_init_count;
            winsock_acquired = true;
        }
    }
    if (startup_error != 0) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                     "WSAStartup failed: " + std::to_string(startup_error));
    }
#endif
}

NetworkClient::~NetworkClient() {
    disconnect();
#ifdef _WIN32
    if (winsock_acquired) {
        std::lock_guard<std::mutex> lock(ws_init_mutex);
        if (ws_init_count > 0 && --ws_init_count == 0) {
            WSACleanup();
            ws_init_ok = false;
        }
        winsock_acquired = false;
    }
#endif
}

bool NetworkClient::connect(const std::string& ip, int port_num) {
    std::lock_guard<std::mutex> socket_lock(socket_mutex);
    // A reconnect replaces the previous descriptor instead of leaking it.
    if (socket_fd != INVALID_SOCKET_VALUE) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
    }
    connected = false;
#ifdef _WIN32
    if (!winsock_acquired) return false;
#endif
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
    
    struct in_addr parsed_address{};
    if (inet_pton(AF_INET, ip.c_str(), &parsed_address) != 1) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_UNREACHABLE, ErrorCategory::NETWORK,
                    "Invalid IPv4 address format: " + ip);
        return false;
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == INVALID_SOCKET_VALUE) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                    "Failed to create socket");
        return false;
    }
    
#ifdef _WIN32
    u_long mode = 1;  // non-blocking
    if (ioctlsocket(socket_fd, FIONBIO, &mode) != 0) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                    "Failed to set non-blocking socket mode");
        return false;
    }
#else
    // Non-blocking connect; both descriptor operations must succeed.
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0 || fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                    "Failed to set non-blocking socket mode");
        return false;
    }
#endif
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(static_cast<u_short>(port));
    
    server_addr.sin_addr = parsed_address;
    
    int result = ::connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
#ifdef _WIN32
    const int connect_error = result < 0 ? WSAGetLastError() : 0;
    if (result < 0 && connect_error != WSAEWOULDBLOCK &&
        connect_error != WSAEINPROGRESS && connect_error != WSAEALREADY) {
#else
    const int connect_error = result < 0 ? errno : 0;
    if (result < 0 && connect_error != EINPROGRESS && connect_error != EWOULDBLOCK) {
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
        const bool timed_out = (result == 0);
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
        REPORT_ERROR(timed_out ? ErrorCode::ERR_NETWORK_TIMEOUT : ErrorCode::ERR_NETWORK_UNREACHABLE,
                    ErrorCategory::NETWORK,
                    timed_out ? "Connection timeout" : "Connection readiness check failed");
        return false;
    }
    
    // Check for connection errors - portable cast for getsockopt
    int so_error = 0;
#ifdef _WIN32
    int len = sizeof(so_error);
    if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len) < 0 || so_error != 0) {
#else
    socklen_t len = sizeof(so_error);
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
    if (ioctlsocket(socket_fd, FIONBIO, &blocking_mode) != 0) {
#else
    int restore_flags = fcntl(socket_fd, F_GETFL, 0);
    if (restore_flags < 0 || fcntl(socket_fd, F_SETFL, restore_flags & ~O_NONBLOCK) < 0) {
#endif
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                    "Failed to restore blocking socket mode");
        return false;
    }
    
    connected = true;
    LOG_INFO("Connected to " + ip + ":" + std::to_string(port));
    return true;
}

void NetworkClient::disconnect() {
    std::lock_guard<std::mutex> socket_lock(socket_mutex);
    if (socket_fd != INVALID_SOCKET_VALUE) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
    }
    connected = false;
    LOG_INFO("Disconnected from device");
}

bool NetworkClient::isConnected() const {
    std::lock_guard<std::mutex> socket_lock(socket_mutex);
    return connected.load() && socket_fd != INVALID_SOCKET_VALUE;
}

bool NetworkClient::sendTherapyPacket(const TherapyPacket& packet) {
    (void)packet;
    // The packed host structure is not a wire protocol. Firmware currently
    // exposes no matching packet receiver, framing, versioning, or ACK path.
    REPORT_ERROR(ErrorCode::ERR_NETWORK_SEND_FAILED, ErrorCategory::NETWORK,
                 "Therapy packet transmission is disabled until RNP is implemented");
    return false;
}

bool NetworkClient::sendCommand(const std::string& cmd) {
    std::lock_guard<std::mutex> socket_lock(socket_mutex);
    if (cmd.empty()) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SEND_FAILED, ErrorCategory::NETWORK,
                    "Cannot send an empty command");
        return false;
    }
    if (!connected.load() || socket_fd == INVALID_SOCKET_VALUE) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_DISCONNECTED, ErrorCategory::NETWORK,
                    "Cannot send command - not connected");
        return false;
    }
    size_t total_sent = 0;
    int send_flags = 0;
#ifndef _WIN32
#ifdef MSG_NOSIGNAL
    send_flags |= MSG_NOSIGNAL;
#endif
#endif
    while (total_sent < cmd.length()) {
#ifdef _WIN32
        int sent = ::send(socket_fd, cmd.data() + total_sent,
                          static_cast<int>(cmd.length() - total_sent), send_flags);
#else
        ssize_t sent = ::send(socket_fd, cmd.data() + total_sent,
                              cmd.length() - total_sent, send_flags);
#endif
        if (sent < 0) {
#ifdef _WIN32
            if (WSAGetLastError() == WSAEINTR) continue;
#else
            if (errno == EINTR) continue;
#endif
        }
        if (sent <= 0) {
            connected = false;
            CLOSE_SOCKET(socket_fd);
            socket_fd = INVALID_SOCKET_VALUE;
            REPORT_ERROR(ErrorCode::ERR_NETWORK_SEND_FAILED, ErrorCategory::NETWORK,
                        "Failed to send complete command; connection closed");
            return false;
        }
        total_sent += static_cast<size_t>(sent);
    }
    return true;
}

std::string NetworkClient::receiveData(int timeout_ms) {
    std::lock_guard<std::mutex> socket_lock(socket_mutex);
    if (!connected.load() || socket_fd == INVALID_SOCKET_VALUE) return "";
    
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
#ifdef _WIN32
    int received = ::recv(socket_fd, buffer, static_cast<int>(sizeof(buffer) - 1), 0);
#else
    ssize_t received = ::recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
#endif
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
    if (received == 0) {
        connected = false;
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
    }
    return "";
}
