/**
 * @file network_client.h
 * @brief Klient sieciowy komunikujący się z Arduino
 */

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <string>
#include <atomic>
#include <mutex>
#include "types.h"

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

class NetworkClient {
private:
    socket_t socket_fd;
#ifdef _WIN32
    struct sockaddr_in server_addr;
#else
    struct sockaddr_in server_addr;
#endif
    std::string ip_address;
    int port;
    std::atomic<bool> connected;
    std::mutex tx_mutex;
    
public:
    NetworkClient();
    ~NetworkClient();
    
    bool connect(const std::string& ip, int port_num);
    void disconnect();
    bool isConnected() const;
    bool sendTherapyPacket(const TherapyPacket& packet);
    bool sendCommand(const std::string& cmd);
    std::string receiveData(int timeout_ms = 100);
    
    std::string getIPAddress() const { return ip_address; }
    int getPort() const { return port; }
};

#endif // NETWORK_CLIENT_H
