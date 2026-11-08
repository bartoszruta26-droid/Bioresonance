/**
 * @file network_system.cpp
 * @brief ENC28J60 TCP diagnostics transport for ResoNet Protocol v1.
 *
 * No actuator/control commands are implemented here.
 */
#include "network_system.h"
#include "network_config.h"
#include "protocol_v1.h"
#include "logging_system.h"
#include "event_system.h"
#include "safety_system.h"

#if RESONET_ENABLE_ETHERNET
#include <SPI.h>
#include <EthernetENC.h>
#endif

static uint8_t net_status = NET_STATUS_DISCONNECTED;
static uint32_t packets_received = 0;
static uint32_t packets_sent = 0;

#if RESONET_ENABLE_ETHERNET
static const uint8_t device_mac[6] = {
    RESONET_MAC_0, RESONET_MAC_1, RESONET_MAC_2,
    RESONET_MAC_3, RESONET_MAC_4, RESONET_MAC_5
};
static EthernetServer diagnostic_server(TCP_PORT);
static EthernetClient diagnostic_client;
static ResoNetProtocolV1::Parser protocol_parser;
static uint32_t last_rx_ms = 0;
static uint32_t last_activity_ms = 0;
static bool server_started = false;
static const uint32_t PARTIAL_FRAME_TIMEOUT_MS = 2000;
static const uint32_t CLIENT_IDLE_TIMEOUT_MS = 30000;
// One request frame is 11 bytes. Bound per-loop work to one frame.
static const uint8_t MAX_RX_BYTES_PER_LOOP = ResoNetProtocolV1::REQUEST_FRAME_SIZE;
static uint8_t rx_buffer[ResoNetProtocolV1::REQUEST_FRAME_SIZE];
static uint8_t tx_buffer[ResoNetProtocolV1::RESPONSE_FRAME_SIZE];

static uint16_t freeMemoryBytes() {
    extern int __heap_start, *__brkval;
    int stack_marker = 0;
    const int heap_end = (__brkval == 0) ? (int)&__heap_start : (int)__brkval;
    const int available = (int)&stack_marker - heap_end;
    if (available <= 0) return 0;
    if (available > 65535) return 65535;
    return (uint16_t)available;
}

static bool sendStatusFrame(uint16_t sequence, const uint8_t* payload) {
    if (!diagnostic_client || !diagnostic_client.connected()) {
        return false;
    }

    const size_t frame_length = ResoNetProtocolV1::encodeStatusResponse(
        sequence, payload, tx_buffer, sizeof(tx_buffer));
    if (frame_length == 0) {
        return false;
    }

    if (diagnostic_client.write(tx_buffer, frame_length) != frame_length) {
        diagnostic_client.stop();
        protocol_parser.reset();
        return false;
    }

    // write(buffer, length) already queues the complete frame; flush is unnecessary.
    ++packets_sent;
    return true;
}

static void handleProtocolFrame(const ResoNetProtocolV1::FrameView& frame,
                                void* context) {
    (void)context;
    ++packets_received;
    last_activity_ms = millis();

    uint8_t payload[ResoNetProtocolV1::STATUS_PAYLOAD_SIZE];
    SafetyStatus* safety = safety_get_status();

    const bool locked = safety != NULL &&
        safety->safe_state == SAFE_STATE_LOCKOUT;
    const bool faulted = safety == NULL ||
        safety->safe_state == SAFE_STATE_ERROR ||
        safety->safe_state == SAFE_STATE_CRITICAL;

    uint16_t fault_flags = 0;
    if (faulted) {
        fault_flags = 0x0001;
    } else if (safety->safe_state == SAFE_STATE_WARNING) {
        fault_flags = 0x0002;
    }

    payload[0] = locked ? 0x05 : (faulted ? 0x04 : 0x01);
    payload[1] = (uint8_t)(fault_flags >> 8);
    payload[2] = (uint8_t)fault_flags;

    const uint32_t uptime_seconds = millis() / 1000UL;
    payload[3] = (uint8_t)(uptime_seconds >> 24);
    payload[4] = (uint8_t)(uptime_seconds >> 16);
    payload[5] = (uint8_t)(uptime_seconds >> 8);
    payload[6] = (uint8_t)uptime_seconds;

    const uint16_t free_memory = freeMemoryBytes();
    payload[7] = (uint8_t)(free_memory >> 8);
    payload[8] = (uint8_t)free_memory;

    // 0x7FFF = no verified external temperature measurement.
    payload[9] = 0x7F;
    payload[10] = 0xFF;
    payload[11] = 0;
    payload[12] = 0;
    // Zero means physical output inhibition is not independently verified.
    payload[13] = 0;

    sendStatusFrame(frame.sequence, payload);
}
#endif

void network_init() {
#if RESONET_ENABLE_ETHERNET
    net_status = NET_STATUS_CONNECTING;
    server_started = false;
    SPI.begin();

    Ethernet.init(PIN_ENC28J60_CS);
    IPAddress ip(RESONET_IP_0, RESONET_IP_1, RESONET_IP_2, RESONET_IP_3);
    IPAddress dns(RESONET_DNS_0, RESONET_DNS_1, RESONET_DNS_2, RESONET_DNS_3);
    IPAddress gateway(RESONET_GATEWAY_0, RESONET_GATEWAY_1,
                      RESONET_GATEWAY_2, RESONET_GATEWAY_3);
    IPAddress subnet(RESONET_SUBNET_0, RESONET_SUBNET_1,
                     RESONET_SUBNET_2, RESONET_SUBNET_3);

    Ethernet.begin(device_mac, ip, dns, gateway, subnet);

    if (Ethernet.hardwareStatus() == EthernetNoHardware ||
        Ethernet.linkStatus() != LinkON) {
        net_status = NET_STATUS_ERROR;
        LOG_ERROR("ENC28J60 unavailable or Ethernet link down");
        EVENT_ERROR(EVENT_NETWORK_DISCONNECTED, "ENC28J60 unavailable");
        return;
    }

    diagnostic_server.begin();
    server_started = true;
    protocol_parser.reset();
    net_status = NET_STATUS_CONNECTED;
    LOG_INFO("Diagnostic TCP server started");
    EVENT_INFO(EVENT_NETWORK_CONNECTED, "Diagnostic TCP server ready");
#else
    net_status = NET_STATUS_DISCONNECTED;
    LOG_INFO("Ethernet transport disabled; configure network_config.h");
#endif
}

void network_loop() {
#if RESONET_ENABLE_ETHERNET
    if (Ethernet.hardwareStatus() == EthernetNoHardware ||
        Ethernet.linkStatus() != LinkON) {
        if (diagnostic_client) diagnostic_client.stop();
        protocol_parser.reset();
        server_started = false;
        net_status = NET_STATUS_ERROR;
        return;
    }

    if (!server_started) {
        diagnostic_server.begin();
        server_started = true;
    }
    net_status = NET_STATUS_CONNECTED;

    if (!diagnostic_client || !diagnostic_client.connected()) {
        if (diagnostic_client) diagnostic_client.stop();
        diagnostic_client = diagnostic_server.accept();
        if (diagnostic_client) {
            protocol_parser.reset();
            last_rx_ms = millis();
            last_activity_ms = last_rx_ms;
        }
    }

    if (!diagnostic_client || !diagnostic_client.connected()) return;

    const int available_bytes = diagnostic_client.available();
    if (available_bytes > 0) {
        const size_t requested = available_bytes > MAX_RX_BYTES_PER_LOOP
            ? MAX_RX_BYTES_PER_LOOP : (size_t)available_bytes;
        const int received = diagnostic_client.read(rx_buffer, requested);
        if (received > 0) {
            last_rx_ms = millis();
            for (int i = 0; i < received; ++i) {
                protocol_parser.feed(rx_buffer[i], handleProtocolFrame, NULL);
            }
        }
    }

    const uint32_t now = millis();
    if (protocol_parser.hasPartialFrame() &&
        now - last_rx_ms > PARTIAL_FRAME_TIMEOUT_MS) {
        protocol_parser.reset();
    }
    if (now - last_activity_ms > CLIENT_IDLE_TIMEOUT_MS) {
        diagnostic_client.stop();
        protocol_parser.reset();
    }
#else
    // Explicitly disabled build: do not claim a network connection.
#endif
}

bool network_is_connected() {
#if RESONET_ENABLE_ETHERNET
    return net_status == NET_STATUS_CONNECTED &&
           Ethernet.linkStatus() == LinkON;
#else
    return false;
#endif
}

uint8_t network_get_status() {
    return net_status;
}

bool network_send_status(const DeviceStatus* status) {
    // Legacy binary status packets are intentionally not transmitted.
    // Status is available only through the versioned STATUS_REQ response.
    (void)status;
    return false;
}

bool network_process_incoming(uint8_t* buffer, size_t len) {
    // Legacy unversioned packets (including therapy/config) are rejected.
    (void)buffer;
    (void)len;
    return false;
}

void network_reset_module() {
#if RESONET_ENABLE_ETHERNET
    if (diagnostic_client) diagnostic_client.stop();
    protocol_parser.reset();
    Ethernet.end();
    server_started = false;
    net_status = NET_STATUS_DISCONNECTED;
    network_init();
#else
    net_status = NET_STATUS_DISCONNECTED;
#endif
}

uint32_t network_get_packets_received() {
    return packets_received;
}

uint32_t network_get_packets_sent() {
    return packets_sent;
}
