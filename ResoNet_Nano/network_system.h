/**
 * @file network_system.h
 * @brief Diagnostyczna warstwa sieciowa ResoNet Protocol v1.
 *
 * Ethernet transport is compile-time disabled by default.
 */
#ifndef NETWORK_SYSTEM_H
#define NETWORK_SYSTEM_H

#include <Arduino.h>
#include "types.h"

#define UDP_PORT 5000
#define TCP_PORT 5001
#define NETWORK_BUFFER_SIZE 203

#define NET_STATUS_DISCONNECTED 0x00
#define NET_STATUS_CONNECTING   0x01
#define NET_STATUS_CONNECTED    0x02
#define NET_STATUS_ERROR        0x03

typedef struct {
    uint32_t current_frequency;
    uint32_t elapsed_time;
    uint8_t status_flags;
    uint8_t error_code;
    uint16_t temperature;
    uint16_t reserved;
} DeviceStatus;

void network_init();
void network_loop();
bool network_is_connected();
uint8_t network_get_status();
bool network_send_status(const DeviceStatus* status);
bool network_process_incoming(uint8_t* buffer, size_t len);
void network_reset_module();
uint32_t network_get_packets_received();
uint32_t network_get_packets_sent();

#endif
