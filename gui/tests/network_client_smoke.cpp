#include "network_client.h"

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
