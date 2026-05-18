/**
 * @file network_system.h
 * @brief System komunikacji sieciowej Ethernet ENC28J60 - Nagłówki
 * @version 4.0 (Modular Architecture with Pseudo-threading)
 */

#ifndef NETWORK_SYSTEM_H
#define NETWORK_SYSTEM_H

#include <Arduino.h>
#include "types.h"  // Import definicji pinów i typów

// ============================================================================
// KONFIGURACJA SIECIOWA (Zgodne z hardware.md)
// ============================================================================

// ENC28J60 Pinout
#define PIN_ENC28J60_CS PIN_ENC28J60_CS
#define PIN_ENC28J60_MOSI PIN_ENC28J60_MOSI
#define PIN_ENC28J60_MISO PIN_ENC28J60_MISO
#define PIN_ENC28J60_SCK PIN_ENC28J60_SCK

#define UDP_PORT 5000
#define TCP_PORT 5001
#define NETWORK_BUFFER_SIZE 512

// Flagi statusu sieci
#define NET_STATUS_DISCONNECTED   0x00
#define NET_STATUS_CONNECTING     0x01
#define NET_STATUS_CONNECTED      0x02
#define NET_STATUS_ERROR          0x03

// Typy pakietów
#define PKT_TYPE_THERAPY          0x01
#define PKT_TYPE_STATUS           0x02
#define PKT_TYPE_CONFIG           0x03
#define PKT_TYPE_ACK              0x04
#define PKT_TYPE_NACK             0x05

// Struktura pakietu terapii (binarny)
typedef struct {
    uint32_t frequency_hz_x100;
    uint32_t duration_sec;
    uint8_t modulation_type;
    uint8_t duty_cycle;
    uint16_t intensity_level;
    uint8_t checksum;
} TherapyPacket;

// Struktura statusu urządzenia
typedef struct {
    uint32_t current_frequency;
    uint32_t elapsed_time;
    uint8_t status_flags;
    uint8_t error_code;
    uint16_t temperature;
    uint16_t reserved;
} DeviceStatus;

// Funkcje publiczne
void network_init();
void network_loop();
bool network_is_connected();
uint8_t network_get_status();
bool network_send_status(const DeviceStatus* status);
bool network_process_incoming(uint8_t* buffer, size_t len);
void network_reset_module();
uint32_t network_get_packets_received();
uint32_t network_get_packets_sent();

#endif // NETWORK_SYSTEM_H
