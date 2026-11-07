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
static const uint8_t MAX_RX_BYTES_PER_LOOP = 24;
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

static bool sendStatusFrame(uint16_t sequence,const uint8_t* payload){
 if(!diagnostic_client||!diagnostic_client.connected())return false;
 size_t n=ResoNetProtocolV1::encodeStatusResponse(sequence,payload,tx_buffer,sizeof(tx_buffer));
 if(!n)return false;
 if(diagnostic_client.write(tx_buffer,n)!=n){diagnostic_client.stop();protocol_parser.reset();return false;}
 ++packets_sent;return true;
}
static void handleProtocolFrame(const ResoNetProtocolV1::FrameView& frame,void* context){
 (void)context;++packets_received;last_activity_ms=millis();
 uint8_t p[ResoNetProtocolV1::STATUS_PAYLOAD_SIZE];SafetyStatus*s=safety_get_status();
 bool locked=s&&s->safe_state==SAFE_STATE_LOCKOUT;
 bool faulted=!s||s->safe_state==SAFE_STATE_ERROR||s->safe_state==SAFE_STATE_CRITICAL;
 uint16_t flags=faulted?1:(s->safe_state==SAFE_STATE_WARNING?2:0);
 p[0]=locked?5:(!faulted?1:4);p[1]=(uint8_t)(flags>>8);p[2]=(uint8_t)flags;
 uint32_t uptime=millis()/1000UL;p[3]=(uint8_t)(uptime>>24);p[4]=(uint8_t)(uptime>>16);p[5]=(uint8_t)(uptime>>8);p[6]=(uint8_t)uptime;
 uint16_t free_mem=freeMemoryBytes();p[7]=(uint8_t)(free_mem>>8);p[8]=(uint8_t)free_mem;
 p[9]=0x7F;p[10]=0xFF;p[11]=0;p[12]=0;p[13]=0;
 sendStatusFrame(frame.sequence,p);
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

    uint8_t processed = 0;
    while (diagnostic_client.available() > 0 &&
           processed < MAX_RX_BYTES_PER_LOOP) {
        const int incoming = diagnostic_client.read();
        if (incoming < 0) break;
        ++processed;
        last_rx_ms = millis();

        const ResoNetProtocolV1::ParseResult result =
            protocol_parser.feed((uint8_t)incoming, handleProtocolFrame, NULL);
        if (result == ResoNetProtocolV1::PARSE_REJECTED) {
            // Invalid data is discarded; no state transition or output action.
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
