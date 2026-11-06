/*
 * ResoNet Diagnostic Nano
 * Minimal protocol image with a RAM-only effector/PWM simulation model.
 * No actuator pin is configured and no physical output can be energized.
 */
#include <Arduino.h>
#include <SPI.h>
#include <EthernetENC.h>
#include "network_config.h"
#include "diagnostic_protocol.h"
#include "effector_runtime.h"

using namespace ResoNetDiagnostic;
static const uint8_t device_mac[6] = { RESONET_DIAG_MAC_0, RESONET_DIAG_MAC_1, RESONET_DIAG_MAC_2, RESONET_DIAG_MAC_3, RESONET_DIAG_MAC_4, RESONET_DIAG_MAC_5 };
static EthernetServer server(RESONET_DIAG_TCP_PORT);
static EthernetClient client;
static Parser parser;
static EffectorRuntime effector_runtime;
static uint8_t tx_buffer[25];
static uint32_t last_rx_ms = 0, last_activity_ms = 0;
static bool server_started = false;

static uint16_t freeMemoryBytes() {
    extern int __heap_start, *__brkval;
    int stack_marker = 0;
    int heap_end = (__brkval == 0) ? (int)&__heap_start : (int)__brkval;
    int available = (int)&stack_marker - heap_end;
    return available > 0 ? static_cast<uint16_t>(available) : 0;
}
static bool sendFrame(uint8_t type, uint16_t sequence, const uint8_t* payload, uint8_t length) {
    if (!client || !client.connected()) return false;
    size_t size = encode(type, sequence, payload, length, tx_buffer, sizeof(tx_buffer));
    if (size == 0 || client.write(tx_buffer, size) != size) {
        client.stop(); parser.reset(); return false;
    }
    return true;
}
static void handleRequest(const Request& request) {
    last_activity_ms = millis();
    if (request.type == HELLO_REQ) {
        uint8_t payload[13] = {'R','N',device_mac[0],device_mac[1],device_mac[2],device_mac[3],device_mac[4],device_mac[5],1,0,0,0,0};
        sendFrame(HELLO_RESP, request.sequence, payload, sizeof(payload)); return;
    }
    if (request.type == PING) { sendFrame(PONG, request.sequence, NULL, 0); return; }
    if (request.type == STATUS_REQ) {
        uint8_t payload[14];
        uint32_t uptime = millis() / 1000UL;
        uint16_t free_memory = freeMemoryBytes();
        payload[0]=0x02; payload[1]=0; payload[2]=0;
        payload[3]=static_cast<uint8_t>(uptime>>24); payload[4]=static_cast<uint8_t>(uptime>>16);
        payload[5]=static_cast<uint8_t>(uptime>>8); payload[6]=static_cast<uint8_t>(uptime);
        payload[7]=static_cast<uint8_t>(free_memory>>8); payload[8]=static_cast<uint8_t>(free_memory);
        payload[9]=0x7F; payload[10]=0xFF; payload[11]=0; payload[12]=0; payload[13]=0;
        sendFrame(STATUS_RESP, request.sequence, payload, sizeof(payload)); return;
    }
    const uint8_t nack[2] = { request.type, 0x02 };
    sendFrame(NACK, request.sequence, nack, sizeof(nack));
}
void setup() {
    SPI.begin(); Ethernet.init(RESONET_DIAG_CS_PIN);
    IPAddress ip(RESONET_DIAG_IP_0,RESONET_DIAG_IP_1,RESONET_DIAG_IP_2,RESONET_DIAG_IP_3);
    IPAddress dns(RESONET_DIAG_DNS_0,RESONET_DIAG_DNS_1,RESONET_DIAG_DNS_2,RESONET_DIAG_DNS_3);
    IPAddress gateway(RESONET_DIAG_GATEWAY_0,RESONET_DIAG_GATEWAY_1,RESONET_DIAG_GATEWAY_2,RESONET_DIAG_GATEWAY_3);
    IPAddress subnet(RESONET_DIAG_SUBNET_0,RESONET_DIAG_SUBNET_1,RESONET_DIAG_SUBNET_2,RESONET_DIAG_SUBNET_3);
    Ethernet.begin(device_mac, ip, dns, gateway, subnet);
    parser.reset(); effector_runtime.clearModel();
    if (Ethernet.hardwareStatus()!=EthernetNoHardware && Ethernet.linkStatus()==LinkON) { server.begin(); server_started=true; }
}
void loop() {
    // Bounded state transition in RAM only; deliberately no GPIO/PWM operation.
    effector_runtime.processOne();
    if (Ethernet.hardwareStatus()==EthernetNoHardware || Ethernet.linkStatus()!=LinkON) {
        if (client) client.stop(); parser.reset(); server_started=false; return;
    }
    if (!server_started) { server.begin(); server_started=true; }
    if (!client || !client.connected()) {
        if (client) client.stop();
        client=server.accept();
        if (client) { parser.reset(); last_rx_ms=millis(); last_activity_ms=last_rx_ms; }
    }
    if (!client || !client.connected()) return;
    uint8_t processed=0;
    while (client.available()>0 && processed<RESONET_DIAG_MAX_RX_PER_LOOP) {
        int value=client.read(); if (value<0) break;
        ++processed; last_rx_ms=millis(); Request request;
        if (parser.feed(static_cast<uint8_t>(value),request)) handleRequest(request);
    }
    uint32_t now=millis();
    if (parser.partial() && now-last_rx_ms>RESONET_DIAG_PARTIAL_TIMEOUT_MS) parser.reset();
    if (now-last_activity_ms>RESONET_DIAG_IDLE_TIMEOUT_MS) { client.stop(); parser.reset(); }
}
