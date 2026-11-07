#include "network_client.h"
#include "error_handler.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using test_socket_t = SOCKET;
static constexpr test_socket_t invalid_test_socket = INVALID_SOCKET;
static void close_test_socket(test_socket_t socket) { if (socket != INVALID_SOCKET) closesocket(socket); }
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
using test_socket_t = int;
static constexpr test_socket_t invalid_test_socket = -1;
static void close_test_socket(test_socket_t socket) { if (socket >= 0) close(socket); }
#endif

static bool send_all(test_socket_t fd, const std::uint8_t* data, std::size_t size) {
    std::size_t offset = 0;
    while (offset < size) {
#ifdef _WIN32
        int n = ::send(fd, reinterpret_cast<const char*>(data + offset), static_cast<int>(size - offset), 0);
        if (n == SOCKET_ERROR) return false;
#else
        ssize_t n = ::send(fd, data + offset, size - offset, MSG_NOSIGNAL);
        if (n < 0 && errno == EINTR) continue;
        if (n < 0) return false;
#endif
        if (n == 0) return false;
        offset += static_cast<std::size_t>(n);
    }
    return true;
}

static bool recv_exact(test_socket_t fd, std::uint8_t* data, std::size_t size) {
    std::size_t offset = 0;
    while (offset < size) {
#ifdef _WIN32
        int n = ::recv(fd, reinterpret_cast<char*>(data + offset), static_cast<int>(size - offset), 0);
        if (n == SOCKET_ERROR) return false;
#else
        ssize_t n = ::recv(fd, data + offset, size - offset, 0);
        if (n < 0 && errno == EINTR) continue;
        if (n < 0) return false;
#endif
        if (n == 0) return false;
        offset += static_cast<std::size_t>(n);
    }
    return true;
}

static bool read_frame(test_socket_t fd, ResoNetV1::Frame& frame) {
    std::uint8_t header[9];
    if (!recv_exact(fd, header, sizeof(header))) return false;
    const std::size_t length = (static_cast<std::size_t>(header[7]) << 8) | header[8];
    if (length > ResoNetV1::kMaxPayload) return false;
    std::vector<std::uint8_t> bytes(header, header + sizeof(header));
    bytes.resize(sizeof(header) + length + 2);
    if (!recv_exact(fd, bytes.data() + sizeof(header), length + 2)) return false;
    return ResoNetV1::decode(bytes, frame);
}

static bool write_frame(test_socket_t fd, std::uint8_t type, std::uint16_t sequence,
                        const std::vector<std::uint8_t>& payload) {
    ResoNetV1::Frame frame;
    frame.type = type;
    frame.sequence = sequence;
    frame.payload = payload;
    std::vector<std::uint8_t> bytes;
    return ResoNetV1::encode(frame, bytes) && send_all(fd, bytes.data(), bytes.size());
}

static test_socket_t make_listener(sockaddr_in& address) {
    test_socket_t listener = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == invalid_test_socket) return invalid_test_socket;
    address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    if (::bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
        ::listen(listener, 1) != 0) {
        close_test_socket(listener);
        return invalid_test_socket;
    }
#ifdef _WIN32
    int length = sizeof(address);
#else
    socklen_t length = sizeof(address);
#endif
    if (getsockname(listener, reinterpret_cast<sockaddr*>(&address), &length) != 0) {
        close_test_socket(listener);
        return invalid_test_socket;
    }
    return listener;
}

static bool serve_status(test_socket_t peer) {
    ResoNetV1::Frame request;
    if (!read_frame(peer, request) || request.type != ResoNetV1::kStatusReq ||
        !request.payload.empty()) return false;
    std::vector<std::uint8_t> status(14, 0);
    status[0] = 1; status[9] = 0x7F; status[10] = 0xFF;
    return write_frame(peer, ResoNetV1::kStatusResp, request.sequence, status);
}

int main() {
#ifdef _WIN32
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return 1;
#endif
    int exit_code = 0;

    // Disconnect must wake a pending raw read after a successful protocol handshake.
    {
        sockaddr_in address{};
        test_socket_t listener = make_listener(address);
        if (listener == invalid_test_socket) exit_code = 2;
        else {
            std::thread server([&]() {
                test_socket_t peer = ::accept(listener, nullptr, nullptr);
                if (peer != invalid_test_socket) {
                    (void)serve_status(peer);
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    close_test_socket(peer);
                }
            });
            NetworkClient client;
            if (!client.connect("127.0.0.1", static_cast<int>(ntohs(address.sin_port)))) exit_code = 3;
            else {
                std::atomic<bool> done{false};
                std::thread reader([&]() { (void)client.receiveData(5000); done = true; });
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                const auto start = std::chrono::steady_clock::now();
                client.disconnect();
                const auto elapsed = std::chrono::steady_clock::now() - start;
                reader.join();
                if (!done || elapsed > std::chrono::milliseconds(1000)) exit_code = 4;
            }
            client.disconnect();
            server.join();
            close_test_socket(listener);
        }
    }

    // STATUS-only protocol must be valid; legacy command APIs fail closed.
    {
        sockaddr_in address{};
        test_socket_t listener = make_listener(address);
        if (listener == invalid_test_socket) exit_code = 5;
        else {
            std::atomic<bool> server_ok{false};
            std::thread server([&]() {
                test_socket_t peer = ::accept(listener, nullptr, nullptr);
                if (peer != invalid_test_socket && serve_status(peer)) {
                    ResoNetV1::Frame request;
                    if (read_frame(peer, request) && request.type == ResoNetV1::kStatusReq &&
                        request.sequence == 2 && request.payload.empty()) {
                        std::vector<std::uint8_t> status(14, 0);
                        status[0] = 1; // IDLE
                        status[6] = 42; // uptime
                        status[8] = 0x80; // 128 bytes free
                        status[9] = 0x7F; status[10] = 0xFF; // no temperature
                        status[13] = 0; // output inhibition unknown
                        server_ok = write_frame(peer, ResoNetV1::kStatusResp, request.sequence, status);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    close_test_socket(peer);
                }
            });
            {
                NetworkClient client;
                if (!client.connect("127.0.0.1", static_cast<int>(ntohs(address.sin_port)))) exit_code = 6;
                else {
                    ResoNetStatusV1 status;
                    if (!client.requestStatus(status) || status.state != 1 ||
                        status.uptime_seconds != 42 || status.free_memory_bytes != 128 ||
                        status.temperature_valid || status.output_inhibited_known) exit_code = 7;
                    if (client.sendCommand("START") || client.sendTherapyPacket(TherapyPacket{}))
                        exit_code = 8;
                }
                client.disconnect();
            }
            server.join();
            close_test_socket(listener);
            if (!server_ok.load() && exit_code == 0) exit_code = 9;
        }
    }

    // A malformed STATUS invalidates and closes the diagnostic session.
    {
        sockaddr_in address{};
        test_socket_t listener = make_listener(address);
        if (listener == invalid_test_socket) exit_code = 10;
        else {
            std::thread server([&]() {
                test_socket_t peer = ::accept(listener, nullptr, nullptr);
                if (peer != invalid_test_socket && serve_status(peer)) {
                    ResoNetV1::Frame request;
                    if (read_frame(peer, request) && request.type == ResoNetV1::kStatusReq) {
                        std::vector<std::uint8_t> status(14, 0);
                        status[0] = 0x7F; // invalid system state
                        (void)write_frame(peer, ResoNetV1::kStatusResp, request.sequence, status);
                    }
                    close_test_socket(peer);
                }
            });
            {
                NetworkClient client;
                if (!client.connect("127.0.0.1", static_cast<int>(ntohs(address.sin_port)))) exit_code = 11;
                else {
                    ResoNetStatusV1 status;
                    if (client.requestStatus(status)) exit_code = 12;
                    if (client.isConnected()) exit_code = 13;
                }
                client.disconnect();
            }
            server.join();
            close_test_socket(listener);
        }
    }


    // Sensor validity flags are reserved in diagnostic v1 and must be zero,
    // matching the Android parser's strict STATUS_RESP validation.
    {
        sockaddr_in address{};
        test_socket_t listener = make_listener(address);
        if (listener == invalid_test_socket) exit_code = 14;
        else {
            std::thread server([&]() {
                test_socket_t peer = ::accept(listener, nullptr, nullptr);
                if (peer != invalid_test_socket && serve_status(peer)) {
                    ResoNetV1::Frame request;
                    if (read_frame(peer, request) && request.type == ResoNetV1::kStatusReq) {
                        std::vector<std::uint8_t> status(14, 0);
                        status[0] = 1; // IDLE
                        status[12] = 1; // Reserved sensor_valid_flags bit
                        (void)write_frame(peer, ResoNetV1::kStatusResp, request.sequence, status);
                    }
                    close_test_socket(peer);
                }
            });
            {
                NetworkClient client;
                if (!client.connect("127.0.0.1", static_cast<int>(ntohs(address.sin_port)))) exit_code = 15;
                else {
                    ResoNetStatusV1 status;
                    if (client.requestStatus(status)) exit_code = 16;
                    if (client.isConnected()) exit_code = 17;
                }
                client.disconnect();
            }
            server.join();
            close_test_socket(listener);
        }
    }


#ifdef _WIN32
    WSACleanup();
#endif
    return exit_code;
}
