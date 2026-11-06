#include "network_client.h"
#include "error_handler.h"

#include <atomic>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>

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
using test_socket_t = int;
static constexpr test_socket_t invalid_test_socket = -1;
static void close_test_socket(test_socket_t socket) { if (socket >= 0) close(socket); }
#endif

int main() {
#ifdef _WIN32
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return 1;
#endif

    int exit_code = 0;

    // disconnect() must wake a thread blocked in receiveData(), rather than
    // waiting for the caller's full receive timeout or racing descriptor close.
    {
        test_socket_t listener = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listener == invalid_test_socket) {
            exit_code = 13;
        } else {
            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            address.sin_port = 0;
            if (::bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
                ::listen(listener, 1) != 0) {
                close_test_socket(listener);
                exit_code = 14;
            } else {
#ifdef _WIN32
                int address_length = sizeof(address);
#else
                socklen_t address_length = sizeof(address);
#endif
                if (getsockname(listener, reinterpret_cast<sockaddr*>(&address), &address_length) != 0) {
                    close_test_socket(listener);
                    exit_code = 15;
                } else {
                    std::thread server([&]() {
                        test_socket_t peer = ::accept(listener, nullptr, nullptr);
                        if (peer != invalid_test_socket) {
                            std::this_thread::sleep_for(std::chrono::seconds(2));
                            close_test_socket(peer);
                        }
                    });
                    NetworkClient client;
                    const int port = static_cast<int>(ntohs(address.sin_port));
                    if (!client.connect("127.0.0.1", port)) {
                        exit_code = 16;
                    } else {
                        std::atomic<bool> receive_finished{false};
                        std::thread receiver([&]() {
                            (void)client.receiveData(5000);
                            receive_finished = true;
                        });
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        const auto disconnect_start = std::chrono::steady_clock::now();
                        client.disconnect();
                        const auto disconnect_elapsed =
                            std::chrono::steady_clock::now() - disconnect_start;
                        receiver.join();
                        if (!receive_finished.load() ||
                            disconnect_elapsed > std::chrono::milliseconds(1000)) {
                            exit_code = 17;
                        }
                    }
                    client.disconnect();
                    server.join();
                    close_test_socket(listener);
                }
            }
        }
    }


    // A pending connection attempt must not hold socket_mutex. disconnect()
    // must also supersede the attempt so it cannot publish a late connection.
    {
        NetworkClient concurrent_client;
        std::atomic<bool> connect_finished{false};
        std::thread connector([&]() {
            (void)concurrent_client.connect("192.0.2.1", 65000);
            connect_finished = true;
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        const auto query_start = std::chrono::steady_clock::now();
        (void)concurrent_client.isConnected();
        const auto query_elapsed = std::chrono::steady_clock::now() - query_start;
        concurrent_client.disconnect();
        connector.join();
        if (query_elapsed > std::chrono::milliseconds(500) ||
            concurrent_client.isConnected()) {
            exit_code = 12;
        }
    }

    {
        test_socket_t listener = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listener == invalid_test_socket) {
            exit_code = 2;
        } else {
            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            address.sin_port = 0;
            if (::bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
                ::listen(listener, 1) != 0) {
                close_test_socket(listener);
                exit_code = 3;
            } else {
#ifdef _WIN32
                int address_length = sizeof(address);
#else
                socklen_t address_length = sizeof(address);
#endif
                if (getsockname(listener, reinterpret_cast<sockaddr*>(&address), &address_length) != 0) {
                    close_test_socket(listener);
                    exit_code = 4;
                } else {
                    std::atomic<bool> server_ok{false};
                    std::thread server([&]() {
                        test_socket_t peer = ::accept(listener, nullptr, nullptr);
                        if (peer != invalid_test_socket) {
                            char request[16]{};
#ifdef _WIN32
                            int count = ::recv(peer, request, static_cast<int>(sizeof(request)), 0);
#else
                            ssize_t count = ::recv(peer, request, sizeof(request), 0);
#endif
                            if (count == 4 && std::string(request, static_cast<std::size_t>(count)) == "ping") {
                                const char response[] = "pong";
#ifdef _WIN32
                                int sent = ::send(peer, response, 4, 0);
#else
                                ssize_t sent = ::send(peer, response, 4, MSG_NOSIGNAL);
#endif
                                server_ok = (sent == 4);
                            }
                            close_test_socket(peer);
                        }
                    });

                    {
                        NetworkClient client;
                        std::atomic<bool> callback_reentered{false};
                        ErrorHandler::getInstance().registerCallback(
                            [&client, &callback_reentered](const ErrorInfo&) {
                                (void)client.isConnected();
                                callback_reentered = true;
                            });
                        // Error callbacks may query the client; never invoke them
                        // while socket_mutex is held.
                        if (client.sendCommand("")) exit_code = 10;
                        if (!callback_reentered.load() && exit_code == 0) exit_code = 11;

                        if (exit_code == 0) {
                            const int port = static_cast<int>(ntohs(address.sin_port));
                            if (!client.connect("127.0.0.1", port)) {
                                exit_code = 5;
                            } else if (!client.sendCommand("ping")) {
                                exit_code = 6;
                            } else if (client.receiveData(2000) != "pong") {
                                exit_code = 7;
                            } else {
                                // The server closes after replying; EOF must clear the client state.
                                for (int attempt = 0; attempt < 20 && client.isConnected(); ++attempt) {
                                    (void)client.receiveData(100);
                                }
                                if (client.isConnected()) exit_code = 8;
                            }
                        }
                        client.disconnect();
                    }

                    server.join();
                    close_test_socket(listener);
                    if (!server_ok.load() && exit_code == 0) exit_code = 9;
                }
            }
        }
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return exit_code;
}
