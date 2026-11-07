/**
 * @file protocol_v1.cpp
 * @brief ResoNet Protocol v1 frame codec and bounded streaming parser.
 */
#include "protocol_v1.h"

namespace ResoNetProtocolV1 {

static uint16_t readU16BE(const uint8_t* p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static void writeU16BE(uint8_t* p, uint16_t value) {
    p[0] = (uint8_t)(value >> 8);
    p[1] = (uint8_t)(value & 0xFF);
}

uint16_t crc16(const uint8_t* data, size_t length) {
    if (data == NULL && length != 0) return 0;
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021)
                                 : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

bool isKnownType(uint8_t type) {
    switch (type) {
        case HELLO_REQ:
        case HELLO_RESP:
        case PING:
        case PONG:
        case STATUS_REQ:
        case STATUS_RESP:
        case NACK:
        case ERROR:
        case ACK:
            return true;
        default:
            return false;
    }
}

bool validatePayloadLength(uint8_t type, uint16_t length) {
    switch (type) {
        case HELLO_REQ:
        case PING:
        case PONG:
        case STATUS_REQ:
            return length == 0;
        case HELLO_RESP:
            return length == 13;
        case STATUS_RESP:
            return length == 14;
        case NACK:
        case ERROR:
            return length == 2;
        case ACK:
            return length == 1;
        default:
            return false;
    }
}

size_t encode(uint8_t type,
              uint16_t sequence,
              const uint8_t* payload,
              uint16_t payload_length,
              uint8_t* output,
              size_t output_capacity) {
    if (output == NULL || payload_length > MAX_PAYLOAD) return 0;
    if (!isKnownType(type) || !validatePayloadLength(type, payload_length)) return 0;
    if (payload_length != 0 && payload == NULL) return 0;

    const size_t frame_size = HEADER_SIZE + payload_length + CRC_SIZE;
    if (output_capacity < frame_size) return 0;

    output[0] = MAGIC_0;
    output[1] = MAGIC_1;
    output[2] = VERSION;
    output[3] = type;
    output[4] = 0;
    writeU16BE(&output[5], sequence);
    writeU16BE(&output[7], payload_length);
    for (uint16_t i = 0; i < payload_length; ++i)
        output[HEADER_SIZE + i] = payload[i];

    writeU16BE(&output[HEADER_SIZE + payload_length],
               crc16(output, HEADER_SIZE + payload_length));
    return frame_size;
}

Parser::Parser()
    : used_(0), expected_size_(0), rejected_frames_(0), state_(SEEK_MAGIC_0) {
}

void Parser::reset() {
    used_ = 0;
    expected_size_ = 0;
    state_ = SEEK_MAGIC_0;
}

void Parser::restartWithByte(uint8_t byte) {
    used_ = 0;
    expected_size_ = 0;
    state_ = SEEK_MAGIC_0;
    if (byte == MAGIC_0) {
        buffer_[0] = byte;
        used_ = 1;
        state_ = SEEK_MAGIC_1;
    }
}

/* Keep a possible magic suffix when rejecting a malformed frame. */
void Parser::resyncFromBuffer() {
    for (uint8_t start = 1; start < used_; ++start) {
        if (buffer_[start] != MAGIC_0) continue;
        if ((uint8_t)(start + 1) < used_ && buffer_[start + 1] != MAGIC_1)
            continue;

        const uint8_t remaining = (uint8_t)(used_ - start);
        for (uint8_t i = 0; i < remaining; ++i)
            buffer_[i] = buffer_[start + i];
        used_ = remaining;
        expected_size_ = 0;
        state_ = remaining == 1 ? SEEK_MAGIC_1 : READ_HEADER;
        return;
    }
    reset();
}

ParseResult Parser::feed(uint8_t byte, FrameHandler handler, void* context) {
    switch (state_) {
        case SEEK_MAGIC_0:
            if (byte == MAGIC_0) {
                buffer_[0] = byte;
                used_ = 1;
                state_ = SEEK_MAGIC_1;
            }
            return PARSE_NEED_MORE;

        case SEEK_MAGIC_1:
            if (byte == MAGIC_1) {
                buffer_[used_++] = byte;
                state_ = READ_HEADER;
            } else {
                restartWithByte(byte);
            }
            return PARSE_NEED_MORE;

        case READ_HEADER:
            if (used_ >= HEADER_SIZE) {
                ++rejected_frames_;
                restartWithByte(byte);
                return PARSE_REJECTED;
            }
            buffer_[used_++] = byte;
            if (used_ == HEADER_SIZE) {
                const uint16_t payload_length = readU16BE(&buffer_[7]);
                const uint8_t type = buffer_[3];
                const bool request_type = type == HELLO_REQ ||
                    type == PING || type == STATUS_REQ;
                if (buffer_[2] != VERSION || buffer_[4] != 0 ||
                    !isKnownType(type) || !request_type ||
                    !validatePayloadLength(type, payload_length) ||
                    payload_length != 0) {
                    ++rejected_frames_;
                    resyncFromBuffer();
                    return PARSE_REJECTED;
                }
                expected_size_ = HEADER_SIZE + CRC_SIZE;
                state_ = READ_BODY;
            }
            return PARSE_NEED_MORE;

        case READ_BODY:
            if (used_ >= MAX_REQUEST_FRAME_SIZE || used_ >= expected_size_) {
                ++rejected_frames_;
                restartWithByte(byte);
                return PARSE_REJECTED;
            }
            buffer_[used_++] = byte;
            if (used_ == expected_size_) {
                const uint16_t received_crc = readU16BE(&buffer_[HEADER_SIZE]);
                const uint16_t calculated_crc = crc16(buffer_, HEADER_SIZE);
                if (received_crc != calculated_crc) {
                    ++rejected_frames_;
                    resyncFromBuffer();
                    return PARSE_REJECTED;
                }

                if (handler != NULL) {
                    FrameView frame;
                    frame.type = buffer_[3];
                    frame.sequence = readU16BE(&buffer_[5]);
                    frame.payload_length = 0;
                    frame.payload = &buffer_[HEADER_SIZE];
                    handler(frame, context);
                }
                reset();
                return PARSE_FRAME_READY;
            }
            return PARSE_NEED_MORE;
    }

    reset();
    ++rejected_frames_;
    return PARSE_REJECTED;
}

} // namespace ResoNetProtocolV1
