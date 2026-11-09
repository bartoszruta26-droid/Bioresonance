#include "usb_serial_diagnostic.h"
#include <Arduino.h>
#include "protocol_v1.h"
#include "safety_system.h"

namespace {
ResoNetProtocolV1::Parser parser;
uint16_t freeMemoryBytes() {
    extern int __heap_start, *__brkval;
    int stack_marker = 0;
    const int heap_end = (__brkval == 0) ? (int)&__heap_start : (int)__brkval;
    const int available = (int)&stack_marker - heap_end;
    if (available <= 0) return 0;
    if (available > 65535) return 65535;
    return (uint16_t)available;
}
void handleFrame(const ResoNetProtocolV1::FrameView& frame, void*) {
    if (frame.type != ResoNetProtocolV1::STATUS_REQ || frame.payload_length != 0) return;
    uint8_t payload[ResoNetProtocolV1::STATUS_PAYLOAD_SIZE] = {0};
    SafetyStatus* safety = safety_get_status();
    const bool locked = safety != NULL && safety->safe_state == SAFE_STATE_LOCKOUT;
    const bool faulted = safety == NULL || safety->safe_state == SAFE_STATE_ERROR ||
                         safety->safe_state == SAFE_STATE_CRITICAL;
    uint16_t fault_flags = faulted ? 0x0001 : 0;
    if (!faulted && safety->safe_state == SAFE_STATE_WARNING) fault_flags = 0x0002;
    payload[0] = locked ? 0x05 : (faulted ? 0x04 : 0x01);
    payload[1] = (uint8_t)(fault_flags >> 8);
    payload[2] = (uint8_t)fault_flags;
    const uint32_t uptime = millis() / 1000UL;
    payload[3] = (uint8_t)(uptime >> 24); payload[4] = (uint8_t)(uptime >> 16);
    payload[5] = (uint8_t)(uptime >> 8); payload[6] = (uint8_t)uptime;
    const uint16_t free_memory = freeMemoryBytes();
    payload[7] = (uint8_t)(free_memory >> 8); payload[8] = (uint8_t)free_memory;
    payload[9] = 0x7F; payload[10] = 0xFF; // no verified external temperature
    payload[11] = 0; payload[12] = 0;      // reserved sensor flags
    payload[13] = 0;                       // physical output inhibit unknown
    uint8_t response[ResoNetProtocolV1::RESPONSE_FRAME_SIZE];
    const size_t length = ResoNetProtocolV1::encodeStatusResponse(
        frame.sequence, payload, response, sizeof(response));
    if (length) Serial.write(response, length);
}
}
void usb_serial_diagnostic_init() { parser.reset(); }
void usb_serial_diagnostic_loop() {
    // Bound work to one request frame per main-loop pass.
    uint8_t budget = ResoNetProtocolV1::REQUEST_FRAME_SIZE;
    while (budget && Serial.available() > 0) {
        const int value = Serial.read();
        if (value < 0) break;
        --budget;
        parser.feed((uint8_t)value, handleFrame, NULL);
    }
}
