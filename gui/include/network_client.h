/**
 * @file network_client.h
 * @brief Klient sieciowy komunikujący się z Arduino
 */

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <string>
#include <atomic>
#include <mutex>
#include <cstdint>
#include "types.h"
#include "resonet_protocol_v1.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <netdb.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VALUE -1
    #define CLOSE_SOCKET close
#endif

struct ResoNetStatusV1 {\n    std::uint8_t state = 0;\n    std::uint16_t fault_flags = 0;\n    std::uint32_t uptime_seconds = 0;\n    std::uint16_t free_memory_bytes = 0;\n    std::int16_t temperature_centi_c = 0;\n    bool temperature_valid = false;\n    std::uint16_t sensor_valid_flags = 0;\n    bool output_inhibited_known = false;\n    bool output_inhibited = false;\n};\n\nclass NetworkClient {
private:
    socket_t socket_fd;
#ifdef _WIN32
    bool winsock_acquired;
#endif
#ifdef _WIN32
    struct sockaddr_in server_addr;
#else
    struct sockaddr_in server_addr;
#endif
    std::string ip_address;
    int port;
    std::atomic<bool> connected;
    mutable std::mutex socket_mutex;
    // Serializes socket I/O and descriptor lifecycle; state mutex is never held during I/O.
    mutable std::mutex io_mutex;
    std::uint64_t connection_generation = 0;\n    std::uint16_t protocol_sequence = 0;
    
public:
    NetworkClient();
    ~NetworkClient();
    
    bool connect(const std::string& ip, int port_num);
    void disconnect();
    bool isConnected() const;\n    bool requestStatus(ResoNetStatusV1& status);
    bool sendTherapyPacket(const TherapyPacket& packet);
    bool sendCommand(const std::string& cmd);
    std::string receiveData(int timeout_ms = 100);
    
    std::string getIPAddress() const {
        std::lock_guard<std::mutex> lock(socket_mutex);
        return ip_address;
    }
    int getPort() const {
        std::lock_guard<std::mutex> lock(socket_mutex);
        return port;
    }
};

#endif // NETWORK_CLIENT_H
