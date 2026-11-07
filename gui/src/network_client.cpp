/**
 * @file network_client.cpp
 * @brief Implementacja klienta sieciowego
 */

#include "network_client.h"
#include "error_handler.h"
#include <cstring>
#include <algorithm>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>


static bool send_all_v1(socket_t fd, const std::uint8_t* data, std::size_t size) {
    std::size_t offset = 0;
    while (offset < size) {
#ifdef _WIN32
        const int sent = ::send(fd, reinterpret_cast<const char*>(data + offset),
                                static_cast<int>(size - offset), 0);
        if (sent == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEINTR) continue;
            return false;
        }
#else
        const ssize_t sent = ::send(fd, data + offset, size - offset,
#ifdef MSG_NOSIGNAL
                                    MSG_NOSIGNAL
#else
                                    0
#endif
        );
        if (sent < 0 && errno == EINTR) continue;
        if (sent < 0) return false;
#endif
        if (sent == 0) return false;
        offset += static_cast<std::size_t>(sent);
    }
    return true;
}

static bool recv_exact_v1(socket_t fd, std::uint8_t* data, std::size_t size) {
    std::size_t offset = 0;
    while (offset < size) {
#ifdef _WIN32
        const int received = ::recv(fd, reinterpret_cast<char*>(data + offset),
                                    static_cast<int>(size - offset), 0);
        if (received == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEINTR) continue;
            return false;
        }
#else
        const ssize_t received = ::recv(fd, data + offset, size - offset, 0);
        if (received < 0 && errno == EINTR) continue;
        if (received < 0) return false;
#endif
        if (received == 0) return false;
        offset += static_cast<std::size_t>(received);
    }
    return true;
}

static bool exchange_v1(socket_t fd, std::uint8_t request_type,
                        std::uint8_t expected_type, std::uint16_t sequence,
                        std::vector<std::uint8_t>& payload) {
    ResoNetV1::Frame request;
    request.type = request_type;
    request.sequence = sequence;
    std::vector<std::uint8_t> encoded;
    if (!ResoNetV1::encode(request, encoded) ||
        !send_all_v1(fd, encoded.data(), encoded.size())) return false;

    std::uint8_t header[ResoNetV1::kHeaderSize];
    if (!recv_exact_v1(fd, header, sizeof(header))) return false;
    if (header[0] != 0x52 || header[1] != 0x4E ||
        header[2] != ResoNetV1::kVersion || header[4] != 0) return false;
    const std::size_t length = (static_cast<std::size_t>(header[7]) << 8) | header[8];
    if (length > ResoNetV1::kMaxPayload) return false;
    std::vector<std::uint8_t> response(header, header + sizeof(header));
    response.resize(sizeof(header) + length + 2);
    if (!recv_exact_v1(fd, response.data() + sizeof(header), length + 2)) return false;
    ResoNetV1::Frame decoded;
    if (!ResoNetV1::decode(response, decoded) || decoded.sequence != sequence ||
        decoded.type != expected_type) return false;
    payload.swap(decoded.payload);
    return true;
}

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

    const std::uint32_t host_order = ntohl(parsed_address.s_addr);
    const bool private_or_loopback =
        (host_order & 0xFF000000UL) == 0x0A000000UL ||
        (host_order & 0xFFF00000UL) == 0xAC100000UL ||
        (host_order & 0xFFFF0000UL) == 0xC0A80000UL ||
        (host_order & 0xFF000000UL) == 0x7F000000UL;
    if (!private_or_loopback) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_UNREACHABLE, ErrorCategory::NETWORK,
                     "Only RFC1918 or loopback IPv4 addresses are permitted");
        return false;
    }

    socket_t candidate = socket(AF_INET, SOCK_STREAM, 0);
    if (candidate == INVALID_SOCKET_VALUE) {
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                     "Failed to create socket");
        return false;
    }
#ifdef _WIN32
    DWORD send_timeout_ms = 2000;
    if (setsockopt(candidate, SOL_SOCKET, SO_SNDTIMEO,
                   reinterpret_cast<const char*>(&send_timeout_ms),
                   sizeof(send_timeout_ms)) != 0) {
#else
    struct timeval send_timeout{};
    send_timeout.tv_sec = 2;
    send_timeout.tv_usec = 0;
    if (setsockopt(candidate, SOL_SOCKET, SO_SNDTIMEO,
                   &send_timeout, sizeof(send_timeout)) != 0) {
#endif
        CLOSE_SOCKET(candidate);
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                     "Failed to configure bounded socket send timeout");
        return false;
    }

#ifdef _WIN32
    DWORD receive_timeout_ms = 2000;
    if (setsockopt(candidate, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&receive_timeout_ms),
                   sizeof(receive_timeout_ms)) != 0) {
#else
    struct timeval receive_timeout{};
    receive_timeout.tv_sec = 2;
    receive_timeout.tv_usec = 0;
    if (setsockopt(candidate, SOL_SOCKET, SO_RCVTIMEO,
                   &receive_timeout, sizeof(receive_timeout)) != 0) {
#endif
        CLOSE_SOCKET(candidate);
        REPORT_ERROR(ErrorCode::ERR_NETWORK_SOCKET_FAILED, ErrorCategory::NETWORK,
                     "Failed to configure bounded socket receive timeout");
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

    // TCP alone is not a verified device connection: require a valid v1 HELLO.
    std::vector<std::uint8_t> hello_payload;
    if (!exchange_v1(candidate, ResoNetV1::kHelloReq, ResoNetV1::kHelloResp,
                     1, hello_payload) || hello_payload.size() != 13 ||
        hello_payload[11] != 0 || hello_payload[12] != 0) {
        close_candidate();
        REPORT_ERROR(ErrorCode::ERR_NETWORK_UNREACHABLE, ErrorCategory::NETWORK,
                     "ResoNet v1 HELLO handshake failed");
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
        protocol_sequence = 1;
    }

    LOG_INFO("Connected to " + ip + ":" + std::to_string(port_num));
    return true;
}

void NetworkClient::disconnect() {
    // shutdown() is safe while another thread is in select/recv/send and wakes it
    // without closing/releasing the descriptor before the I/O owner has exited.
    {
        std::lock_guard<std::mutex> state_lock(socket_mutex);
        ++connection_generation;
        connected = false;
        if (socket_fd != INVALID_SOCKET_VALUE) {
#ifdef _WIN32
            ::shutdown(socket_fd, SD_BOTH);
#else
            ::shutdown(socket_fd, SHUT_RDWR);
#endif
        }
    }

    std::lock_guard<std::mutex> io_lock(io_mutex);
    std::lock_guard<std::mutex> state_lock(socket_mutex);
    if (socket_fd != INVALID_SOCKET_VALUE) {
        CLOSE_SOCKET(socket_fd);
        socket_fd = INVALID_SOCKET_VALUE;
    }
    LOG_INFO("Disconnected from device");
}

bool NetworkClient::isConnected() const {
    std::unique_lock<std::mutex> socket_lock(socket_mutex);
    return connected.load() && socket_fd != INVALID_SOCKET_VALUE;
}

bool NetworkClient::requestStatus(ResoNetStatusV1& status) {
    std::unique_lock<std::mutex> io_lock(io_mutex);
    socket_t active_socket = INVALID_SOCKET_VALUE;
    std::uint16_t sequence = 0;
    {
        std::lock_guard<std::mutex> state_lock(socket_mutex);
        if (!connected.load() || socket_fd == INVALID_SOCKET_VALUE) return false;
        active_socket = socket_fd;
        protocol_sequence = static_cast<std::uint16_t>(protocol_sequence + 1);
        sequence = protocol_sequence;
    }

    std::vector<std::uint8_t> payload;
    if (!exchange_v1(active_socket, ResoNetV1::kStatusReq,
                     ResoNetV1::kStatusResp, sequence, payload) || payload.size() != 14 ||
        payload[0] > 5 || (((payload[1] << 8) | payload[2]) & ~0x0003) != 0 ||
        payload[13] > 1) {
        std::lock_guard<std::mutex> state_lock(socket_mutex);
        if (socket_fd == active_socket) connected = false;
        return false;
    }
    const auto u16 = [&payload](std::size_t i) {
        return static_cast<std::uint16_t>((payload[i] << 8) | payload[i + 1]);
    };
    const auto u32 = [&payload](std::size_t i) {
        return (static_cast<std::uint32_t>(payload[i]) << 24) |
               (static_cast<std::uint32_t>(payload[i + 1]) << 16) |
               (static_cast<std::uint32_t>(payload[i + 2]) << 8) |
               static_cast<std::uint32_t>(payload[i + 3]);
    };
    status.state = payload[0];
    status.fault_flags = u16(1);
    status.uptime_seconds = u32(3);
    status.free_memory_bytes = u16(7);
    status.temperature_centi_c = static_cast<std::int16_t>(u16(9));
    status.temperature_valid = status.temperature_centi_c != 0x7FFF;
    status.sensor_valid_flags = u16(11);
    status.output_inhibited_known = payload[13] == 1;
    status.output_inhibited = payload[13] == 1;
    return true;
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
    (void)cmd;
    // Protocol v1 is diagnostic-only; raw text commands are never transmitted.
    return false;
}

std::string NetworkClient::receiveData(int timeout_ms) {
    if (timeout_ms < 0) {
        REPORT_ERROR(ErrorCode::ERR_USER_INPUT_INVALID, ErrorCategory::USER,
                     "Receive timeout must be non-negative");
        return "";
    }
    // Keep disconnect latency bounded even if a caller requests an excessive wait.
    timeout_ms = std::min(timeout_ms, 5000);

    std::unique_lock<std::mutex> io_lock(io_mutex);
    socket_t active_socket = INVALID_SOCKET_VALUE;
    {
        std::lock_guard<std::mutex> state_lock(socket_mutex);
        if (!connected.load() || socket_fd == INVALID_SOCKET_VALUE) return "";
        active_socket = socket_fd;
    }

    // Poll in short slices so disconnect() can cancel the wait on platforms
    // where shutdown() does not reliably wake select() in another thread.
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(timeout_ms);
    int result = 0;
    bool ready = false;
    do {
        if (!connected.load()) return "";

        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::microseconds>(
            deadline - now);
        if (timeout_ms != 0 && remaining.count() <= 0) return "";

        const auto slice = timeout_ms == 0
            ? std::chrono::microseconds(0)
            : std::min(remaining, std::chrono::microseconds(50000));
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(active_socket, &read_fds);
        struct timeval tv{};
        tv.tv_sec = static_cast<long>(slice.count() / 1000000);
        tv.tv_usec = static_cast<long>(slice.count() % 1000000);

#ifdef _WIN32
        result = select(0, &read_fds, nullptr, nullptr, &tv);
        if (result < 0 && WSAGetLastError() == WSAEINTR) continue;
#else
        result = select(active_socket + 1, &read_fds, nullptr, nullptr, &tv);
        if (result < 0 && errno == EINTR) continue;
#endif
        if (result > 0) {
            ready = true;
            break;
        }
        if (result < 0) break;
        if (timeout_ms == 0) return "";
    } while (connected.load());

    if (!connected.load()) return "";
    if (result == 0 || !ready) {
        if (result == 0) return "";
    }

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
