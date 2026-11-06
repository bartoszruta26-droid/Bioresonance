/**
 * @file protocol_v1.h
 * @brief Bounded, allocation-free ResoNet Protocol v1 framing and CRC.
 *
 * Diagnostic protocol only. This module does not expose actuator commands.
 * The payload pointer passed to the handler is valid only during the callback.
 */
#ifndef RESONET_PROTOCOL_V1_H
#define RESONET_PROTOCOL_V1_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

namespace ResoNetProtocolV1 {

static const uint8_t VERSION = 0x01;
static const uint8_t MAGIC_0 = 0x52;
static const uint8_t MAGIC_1 = 0x4E;
static const uint16_t MAX_PAYLOAD = 192;
static const uint16_t HEADER_SIZE = 9;
static const uint16_t CRC_SIZE = 2;
static const uint16_t MAX_FRAME_SIZE = HEADER_SIZE + MAX_PAYLOAD + CRC_SIZE;

enum MessageType : uint8_t {
    HELLO_REQ = 0x01,
    HELLO_RESP = 0x02,
    PING = 0x03,
    PONG = 0x04,
    STATUS_REQ = 0x05,
    STATUS_RESP = 0x06,
    NACK = 0x7D,
    ERROR = 0x7E,
    ACK = 0x7F
};

enum ParseResult : uint8_t {
    PARSE_NEED_MORE = 0,
    PARSE_FRAME_READY,
    PARSE_REJECTED
};

struct FrameView {
    uint8_t type;
    uint16_t sequence;
    uint16_t payload_length;
    const uint8_t* payload;
};

typedef void (*FrameHandler)(const FrameView& frame, void* context);

uint16_t crc16(const uint8_t* data, size_t length);
bool isKnownType(uint8_t type);
bool validatePayloadLength(uint8_t type, uint16_t length);

/**
 * Encode a complete frame into caller-owned storage.
 * Returns 0 on invalid input or insufficient capacity.
 */
size_t encode(uint8_t type,
              uint16_t sequence,
              const uint8_t* payload,
              uint16_t payload_length,
              uint8_t* output,
              size_t output_capacity);

/**
 * Incremental TCP-stream parser. One instance consumes one byte stream.
 * No heap allocation, String, STL containers, or blocking operations.
 */
class Parser {
public:
    Parser();
    void reset();

    ParseResult feed(uint8_t byte, FrameHandler handler, void* context);

    uint16_t rejectedFrames() const { return rejected_frames_; }

private:
    enum State : uint8_t {
        SEEK_MAGIC_0,
        SEEK_MAGIC_1,
        READ_HEADER,
        READ_BODY
    };

    uint8_t buffer_[MAX_FRAME_SIZE];
    uint16_t used_;
    uint16_t expected_size_;
    uint16_t rejected_frames_;
    State state_;

    void restartWithByte(uint8_t byte);
};

} // namespace ResoNetProtocolV1

#endif
