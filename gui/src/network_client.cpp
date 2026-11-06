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
    // Invalidate any previous connection attempt and detach the old socket quickly.
    // The potentially blocking network handshake uses a private local socket.
    std::uint64_t attempt_generation = 0;
    {
        std::lock_guard<std::mutex> io_lock(io_mutex);
        std::lock_guard<std::mutex> lock(socket_mutex);
        attempt_generation = ++connection_generation;
        if (socket_fd != INVALID_SOCKET_VALUE) {
            CLOSE_SOCKET(socket_fd);
            socket_fd = INVALID_SOCKET_VALUE;
        }
        connected = false;
    }

#ifdef _WIN32
    if (!winsock_acquired) return false;
#endif
    if (ip.empty()) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_UNREACHABLE, ErrorCategory::NETWORK,
                     "Empty IP address");
        return false;
    }
    if (port_num < 1 || port_num > 65535) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                     "Invalid port number: " + std::to_string(port_num));
        return false;
    }

    struct in_addr parsed_address{};
    if (inet_pton(AF_INET, ip.c_str(), &parsed_address) != 1) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_UNREACHABLE, ErrorCategory::NETWORK,
                     "Invalid IPv4 address format: " + ip);
        return false;
    }

    socket_t candidate = socket(AF_INET, SOCK_STREAM, 0);
    if (candidate == INVALID_SOCKET_VALUE) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                     "Failed to create socket");
        return false;
    }
    const auto close_candidate = [&candidate]() {
        if (candidate != INVALID_SOCKET_VALUE) {
            CLOSE_SOCKET(candidate);
            candidate = INVALID_SOCKET_VALUE;
        }
    };

#ifdef _WIN32
    u_long mode = 1;
    if (ioctlsocket(candidate, FIONBIO, &mode) != 0) {
#else
    int flags = fcntl(candidate, F_GETFL, 0);
    if (flags < 0 || fcntl(candidate, F_SETFL, flags | O_NONBLOCK) < 0) {
#endif
        close_candidate();
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                     "Failed to set non-blocking socket mode");
        return false;
    }

    struct sockaddr_in candidate_address{};
    candidate_address.sin_family = AF_INET;
    candidate_address.sin_port = htons(static_cast<u_short>(port_num));
    candidate_address.sin_addr = parsed_address;

    int result = ::connect(candidate, reinterpret_cast<struct sockaddr*>(&candidate_address),
                           sizeof(candidate_address));
#ifdef _WIN32
    const int connect_error = result < 0 ? WSAGetLastError() : 0;
    if (result < 0 && connect_error != WSAEWOULDBLOCK &&
        connect_error != WSAEINPROGRESS && connect_error != WSAEALREADY) {
#else
    const int connect_error = result < 0 ? errno : 0;
    if (result < 0 && connect_error != EINPROGRESS && connect_error != EWOULDBLOCK) {
#endif
        close_candidate();
        REPORT_ERROR(ErrorCode::ERR_NETWORK_UNREACHABLE, ErrorCategory::NETWORK,
                     "Connection failed");
        return false;
    }

    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(candidate, &write_fds);
    struct timeval timeout{};
    timeout.tv_sec = 5;
#ifdef _WIN32
    result = select(0, nullptr, &write_fds, nullptr, &timeout);
#else
    result = select(candidate + 1, nullptr, &write_fds, nullptr, &timeout);
#endif
    if (result <= 0) {
        const bool timed_out = (result == 0);
        close_candidate();
        REPORT_ERROR(timed_out ? ErrorCode::ERR_NETWORK_TIMEOUT : ErrorCode::ERR_NETWORK_UNREACHABLE,
                     ErrorCategory::NETWORK,
                     timed_out ? "Connection timeout" : "Connection readiness check failed");
        return false;
    }

    int socket_error = 0;
#ifdef _WIN32
    int error_length = sizeof(socket_error);
    const int option_result = getsockopt(candidate, SOL_SOCKET, SO_ERROR,
                                         reinterpret_cast<char*>(&socket_error), &error_length);
#else
    socklen_t error_length = sizeof(socket_error);
    const int option_result = getsockopt(candidate, SOL_SOCKET, SO_ERROR,
                                         &socket_error, &error_length);
#endif
    if (option_result < 0 || socket_error != 0) {
        close_candidate();
        REPORT_ERROR(ErrorCode::ERR_NETWORK_UNREACHABLE, ErrorCategory::NETWORK,
                     "Connection error: " + std::to_string(socket_error));
        return false;
    }

#ifdef _WIN32
    u_long blocking_mode = 0;
    if (ioctlsocket(candidate, FIONBIO, &blocking_mode) != 0) {
#else
    const int restore_flags = fcntl(candidate, F_GETFL, 0);
    if (restore_flags < 0 || fcntl(candidate, F_SETFL, restore_flags & ~O_NONBLOCK) < 0) {
#endif
        close_candidate();
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                     "Failed to restore blocking socket mode");
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(socket_mutex);
        // disconnect() or a newer connect() supersedes this attempt.
        if (connection_generation != attempt_generation) {
            close_candidate();
            return false;
        }
        socket_fd = candidate;
        candidate = INVALID_SOCKET_VALUE;
        ip_address = ip;
        port = port_num;
        connected = true;
    }

    LOG_INFO("Connected to " + ip + ":" + std::to_string(port_num));
    return true;
}

void NetworkClient::disconnect() {
    std::lock_guard<std::mutex> io_lock(io_mutex);
    std::unique_lock<std::mutex> socket_lock(socket_mutex);
    ++connection_generation;
    if (socket_fd != INVALID_SOCKET_VALUE) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
    }
    connected = false;
    LOG_INFO("Disconnected from device");
}

bool NetworkClient::isConnected() const {
    std::unique_lock<std::mutex> socket_lock(socket_mutex);
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
    if (cmd.empty()) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SEND_FAILED, ErrorCategory::NETWORK,
                     "Cannot send an empty command");
        return false;
    }

    std::unique_lock<std::mutex> io_lock(io_mutex);
    socket_t active_socket = INVALID_SOCKET_VALUE;
    {
        std::lock_guard<std::mutex> state_lock(socket_mutex);
        if (!connected.load() || socket_fd == INVALID_SOCKET_VALUE) {
            io_lock.unlock();
            REPORT_ERROR(ErrorCode::ERR_NETWORK_DISCONNECTED, ErrorCategory::NETWORK,
                         "Cannot send command - not connected");
            return false;
        }
        active_socket = socket_fd;
    }

    size_t total_sent = 0;
    int send_flags = 0;
#ifndef _WIN32
#ifdef MSG_NOSIGNAL
    send_flags |= MSG_NOSIGNAL;
#endif
#endif
    while (total_sent < cmd.size()) {
#ifdef _WIN32
        int sent = ::send(active_socket, cmd.data() + total_sent,
                          static_cast<int>(cmd.size() - total_sent), send_flags);
#else
        ssize_t sent = ::send(active_socket, cmd.data() + total_sent,
                              cmd.size() - total_sent, send_flags);
#endif
        if (sent < 0) {
#ifdef _WIN32
            if (WSAGetLastError() == WSAEINTR) continue;
#else
            if (errno == EINTR) continue;
#endif
        }
        if (sent <= 0) {
            {
                std::lock_guard<std::mutex> state_lock(socket_mutex);
                if (socket_fd == active_socket) {
                    connected = false;
                    CLOSE_SOCKET(socket_fd);
                    socket_fd = INVALID_SOCKET_VALUE;
                    ++connection_generation;
                }
            }
            io_lock.unlock();
            REPORT_ERROR(ErrorCode::ERR_NETWORK_SEND_FAILED, ErrorCategory::NETWORK,
                         "Failed to send complete command; connection closed");
            return false;
        }
        total_sent += static_cast<size_t>(sent);
    }
    return true;
}

std::string NetworkClient::receiveData(int timeout_ms) {
    if (timeout_ms < 0) {
        REPORT_ERROR(ErrorCode::ERR_USER_INPUT_INVALID, ErrorCategory::USER,
                     "Receive timeout must be non-negative");
        return "";
    }

    std::unique_lock<std::mutex> io_lock(io_mutex);
    socket_t active_socket = INVALID_SOCKET_VALUE;
    {
        std::lock_guard<std::mutex> state_lock(socket_mutex);
        if (!connected.load() || socket_fd == INVALID_SOCKET_VALUE) return "";
        active_socket = socket_fd;
    }

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(active_socket, &read_fds);
    struct timeval tv{};
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

#ifdef _WIN32
    int result = select(0, &read_fds, nullptr, nullptr, &tv);
    if (result < 0 && WSAGetLastError() == WSAEINTR) return "";
#else
    int result = select(active_socket + 1, &read_fds, nullptr, nullptr, &tv);
    if (result < 0 && errno == EINTR) return "";
#endif
    if (result == 0) return "";

    auto close_failed_connection = [&]() {
        std::lock_guard<std::mutex> state_lock(socket_mutex);
        if (socket_fd == active_socket) {
            connected = false;
            CLOSE_SOCKET(socket_fd);
            socket_fd = INVALID_SOCKET_VALUE;
            ++connection_generation;
        }
    };

    if (result < 0) {
        close_failed_connection();
        io_lock.unlock();
        REPORT_ERROR(ErrorCode::ERR_NETWORK_RECV_FAILED, ErrorCategory::NETWORK,
                     "Socket readiness check failed; connection closed");
        return "";
    }

    char buffer[1024];
#ifdef _WIN32
    int received = ::recv(active_socket, buffer, static_cast<int>(sizeof(buffer)), 0);
#else
    ssize_t received = ::recv(active_socket, buffer, sizeof(buffer), 0);
#endif
    if (received < 0) {
#ifdef _WIN32
        const int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK || err == WSAEINTR) return "";
#else
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) return "";
#endif
        close_failed_connection();
        io_lock.unlock();
        REPORT_ERROR(ErrorCode::ERR_NETWORK_RECV_FAILED, ErrorCategory::NETWORK,
                     "Receive failed; connection closed");
        return "";
    }
    if (received == 0) {
        close_failed_connection();
        return "";
    }
    return std::string(buffer, static_cast<size_t>(received));
}
