#ifndef RESONET_DIAGNOSTIC_PROTOCOL_H
#define RESONET_DIAGNOSTIC_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

namespace ResoNetDiagnostic {
static const uint8_t VERSION = 1;
static const uint8_t HELLO_REQ = 0x01;
static const uint8_t HELLO_RESP = 0x02;
static const uint8_t PING = 0x03;
static const uint8_t PONG = 0x04;
static const uint8_t STATUS_REQ = 0x05;
static const uint8_t STATUS_RESP = 0x06;
static const uint8_t NACK = 0x7D;

struct Request {
    uint8_t type;
    uint16_t sequence;
};

class Parser {
public:
    Parser() : state_(0), used_(0) {}

    void reset() { state_ = 0; used_ = 0; }
    bool partial() const { return state_ != 0; }

    // Returns true only for a complete, CRC-validated empty-payload request.
    bool feed(uint8_t byte, Request& request) {
        switch (state_) {
        case 0:
            if (byte == 0x52) { buffer_[0] = byte; used_ = 1; state_ = 1; }
            return false;
        case 1:
            if (byte == 0x4E) { buffer_[used_++] = byte; state_ = 2; }
            else restart(byte);
            return false;
        case 2:
            if (used_ >= 9) { restart(byte); return false; }
            buffer_[used_++] = byte;
            if (used_ == 9) {
                const uint16_t length = static_cast<uint16_t>((buffer_[7] << 8) | buffer_[8]);
                if (buffer_[2] != VERSION || buffer_[4] != 0 || length != 0) {
                    restart(byte);
                    return false;
                }
                state_ = 3;
            }
            return false;
        case 3:
            buffer_[used_++] = byte;
            state_ = 4;
            return false;
        case 4: {
            buffer_[used_++] = byte;
            const uint16_t received = static_cast<uint16_t>((buffer_[9] << 8) | buffer_[10]);
            const uint16_t calculated = crc16(buffer_, 9);
            const bool valid = received == calculated;
            if (valid && (buffer_[3] == HELLO_REQ || buffer_[3] == PING || buffer_[3] == STATUS_REQ)) {
                request.type = buffer_[3];
                request.sequence = static_cast<uint16_t>((buffer_[5] << 8) | buffer_[6]);
            }
            reset();
            return valid && (request.type == HELLO_REQ || request.type == PING || request.type == STATUS_REQ);
        }
        default:
            reset();
            return false;
        }
    }

    static uint16_t crc16(const uint8_t* data, size_t length) {
        uint16_t crc = 0xFFFF;
        for (size_t i = 0; i < length; ++i) {
            crc ^= static_cast<uint16_t>(data[i]) << 8;
            for (uint8_t bit = 0; bit < 8; ++bit)
                crc = (crc & 0x8000) ? static_cast<uint16_t>((crc << 1) ^ 0x1021)
                                     : static_cast<uint16_t>(crc << 1);
        }
        return crc;
    }

private:
    uint8_t buffer_[11];
    uint8_t state_;
    uint8_t used_;

    void restart(uint8_t byte) {
        state_ = 0;
        used_ = 0;
        if (byte == 0x52) { buffer_[0] = byte; used_ = 1; state_ = 1; }
    }

};

inline size_t encode(uint8_t type, uint16_t sequence, const uint8_t* payload,
                     uint8_t payload_length, uint8_t* output, size_t capacity) {
    const size_t total = 11U + payload_length;
    if (payload_length > 14 || output == NULL ||
        (payload_length != 0 && payload == NULL) || capacity < total)
        return 0;
    output[0] = 0x52; output[1] = 0x4E; output[2] = VERSION; output[3] = type;
    output[4] = 0;
    output[5] = static_cast<uint8_t>(sequence >> 8);
    output[6] = static_cast<uint8_t>(sequence);
    output[7] = 0;
    output[8] = payload_length;
    for (uint8_t i = 0; i < payload_length; ++i) output[9 + i] = payload[i];
    const uint16_t crc = Parser::crc16(output, 9U + payload_length);
    output[9 + payload_length] = static_cast<uint8_t>(crc >> 8);
    output[10 + payload_length] = static_cast<uint8_t>(crc);
    return total;
}
} // namespace ResoNetDiagnostic

#endif
