/**
 * @file network_system.cpp
 * @brief System komunikacji sieciowej Ethernet ENC28J60 - Implementacja
 * @version 4.0 (Modular Architecture with Pseudo-threading)
 */

#include "network_system.h"
#include "logging_system.h"
#include "event_system.h"
#include <SPI.h>

// ============================================================================
// ZMIENNE PRYWATNE
// ============================================================================

static uint8_t net_status = NET_STATUS_DISCONNECTED;
static uint32_t packets_received = 0;
static uint32_t packets_sent = 0;
static uint32_t last_heartbeat = 0;
static uint32_t last_network_check = 0;

// Piny ENC28J60
#define ENC28J60_CS   10

// Bufory
static uint8_t rx_buffer[NETWORK_BUFFER_SIZE];
static uint8_t tx_buffer[NETWORK_BUFFER_SIZE];

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

/**
 * @brief Oblicza CRC8 dla pakietu
 */
static uint8_t calculate_crc8(const uint8_t* data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/**
 * @brief Symuluje inicjalizację ENC28J60 (stub)
 */
static bool enc28j60_init() {
    // W prawdziwej implementacji tutaj byłaby inicjalizacja ENC28J60
    pinMode(ENC28J60_CS, OUTPUT);
    digitalWrite(ENC28J60_CS, HIGH);
    
    LOG_INFO("ENC28J60 module initialized (stub)");
    return true;
}

/**
 * @brief Sprawdza połączenie sieciowe
 */
static bool check_network_link() {
    // Stub - w rzeczywistości sprawdzałoby link status z ENC28J60
    return net_status == NET_STATUS_CONNECTED;
}

// ============================================================================
// IMPLEMENTACJA FUNKCJI PUBLICZNYCH
// ============================================================================

/**
 * @brief Inicjalizuje system sieciowy
 */
void network_init() {
    net_status = NET_STATUS_CONNECTING;
    
    // Inicjalizuj SPI
    SPI.begin();
    
    // Inicjalizuj moduł ENC28J60
    if (enc28j60_init()) {
        net_status = NET_STATUS_CONNECTED;
        EVENT_INFO(EVENT_NETWORK_CONNECTED, "Network connected");
        LOG_INFO("Network system initialized");
    } else {
        net_status = NET_STATUS_ERROR;
        EVENT_ERROR(EVENT_NETWORK_DISCONNECTED, "Network init failed");
        LOG_ERROR("Network initialization failed");
    }
    
    last_heartbeat = millis();
    last_network_check = millis();
}

/**
 * @brief Główna pętla systemu sieciowego (pseudo-wątek)
 */
void network_loop() {
    uint32_t now = millis();
    
    // Sprawdź połączenie co 5 sekund
    if (now - last_network_check >= 5000) {
        last_network_check = now;
        
        if (net_status == NET_STATUS_CONNECTED && !check_network_link()) {
            net_status = NET_STATUS_ERROR;
            EVENT_WARNING(EVENT_NETWORK_DISCONNECTED, "Network link lost");
            LOG_WARN("Network link lost");
            
            // Spróbuj zresetować moduł
            network_reset_module();
        }
    }
    
    // Aktualizuj heartbeat jeśli połączony
    if (net_status == NET_STATUS_CONNECTED) {
        last_heartbeat = now;
    }
    
    // Tutaj byłoby przetwarzanie przychodzących pakietów
    // W wersji stub pomijamy
}

/**
 * @brief Sprawdza czy sieć jest podłączona
 */
bool network_is_connected() {
    return (net_status == NET_STATUS_CONNECTED);
}

/**
 * @brief Pobiera status sieci
 */
uint8_t network_get_status() {
    return net_status;
}

/**
 * @brief Wysyła status urządzenia przez UDP
 */
bool network_send_status(const DeviceStatus* status) {
    if (net_status != NET_STATUS_CONNECTED) {
        return false;
    }
    
    if (!status) return false;
    
    // Przygotuj pakiet
    tx_buffer[0] = PKT_TYPE_STATUS;
    memcpy(&tx_buffer[1], status, sizeof(DeviceStatus));
    
    // Oblicz CRC
    tx_buffer[1 + sizeof(DeviceStatus)] = calculate_crc8(&tx_buffer[1], sizeof(DeviceStatus));
    
    // W prawdziwej implementacji: wysyłka przez UDP
    // stub: tylko symulacja
    packets_sent++;
    
    return true;
}

/**
 * @brief Przetwarza przychodzące dane
 */
bool network_process_incoming(uint8_t* buffer, size_t len) {
    if (!buffer || len < 2) return false;
    
    packets_received++;
    
    uint8_t pkt_type = buffer[0];
    
    switch (pkt_type) {
        case PKT_TYPE_THERAPY:
            if (len >= sizeof(TherapyPacket) + 1) {
                TherapyPacket* pkt = (TherapyPacket*)&buffer[1];
                
                // Zweryfikuj CRC
                uint8_t expected_crc = calculate_crc8(&buffer[1], sizeof(TherapyPacket) - 1);
                if (pkt->checksum != expected_crc) {
                    LOG_ERROR("CRC check failed for therapy packet");
                    return false;
                }
                
                EVENT_DATA(EVENT_PACKET_RECEIVED, pkt->frequency_hz_x100, "Therapy packet received");
                return true;
            }
            break;
            
        case PKT_TYPE_CONFIG:
            EVENT_INFO(EVENT_PACKET_RECEIVED, "Config packet received");
            return true;
            
        default:
            LOG_WARN_F("Unknown packet type: 0x%02X", pkt_type);
            break;
    }
    
    return false;
}

/**
 * @brief Resetuje moduł sieciowy
 */
void network_reset_module() {
    LOG_INFO("Resetting network module...");
    
    digitalWrite(ENC28J60_CS, LOW);
    delay(10);
    digitalWrite(ENC28J60_CS, HIGH);
    delay(100);
    
    // Spróbuj ponownie zainicjalizować
    if (enc28j60_init()) {
        net_status = NET_STATUS_CONNECTED;
        EVENT_INFO(EVENT_NETWORK_CONNECTED, "Network reconnected after reset");
    } else {
        net_status = NET_STATUS_ERROR;
    }
}

/**
 * @brief Pobiera liczbę odebranych pakietów
 */
uint32_t network_get_packets_received() {
    return packets_received;
}

/**
 * @brief Pobiera liczbę wysłanych pakietów
 */
uint32_t network_get_packets_sent() {
    return packets_sent;
}
