#ifndef RESONET_PROTOCOL_V1_H
#define RESONET_PROTOCOL_V1_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace ResoNetV1 {
constexpr std::uint8_t kVersion = 1;
constexpr std::size_t kHeaderSize = 9;
constexpr std::size_t kMaxPayload = 192;
constexpr std::size_t kMaxFrame = kHeaderSize + kMaxPayload + 2;
constexpr std::uint8_t kHelloReq = 0x01, kHelloResp = 0x02;
constexpr std::uint8_t kPing = 0x03, kPong = 0x04;
constexpr std::uint8_t kStatusReq = 0x05, kStatusResp = 0x06;
constexpr std::uint8_t kNack = 0x7D, kError = 0x7E, kAck = 0x7F;

struct Frame {
    std::uint8_t type = 0;
    std::uint16_t sequence = 0;
    std::vector<std::uint8_t> payload;
};

inline bool isKnownType(std::uint8_t type) {
    switch (type) {
        case kHelloReq: case kHelloResp: case kPing: case kPong:
        case kStatusReq: case kStatusResp: case kNack: case kError: case kAck:
            return true;
        default:
            return false;
    }
}

inline bool isValidPayloadLength(std::uint8_t type, std::size_t length) {
    switch (type) {
        case kHelloReq: case kPing: case kPong: case kStatusReq:
            return length == 0;
        case kHelloResp: return length == 13;
        case kStatusResp: return length == 14;
        case kNack: case kError: return length == 2;
        case kAck: return length == 1;
        default: return false;
    }
}

inline std::uint16_t crc16(const std::uint8_t* data, std::size_t size) {
    std::uint16_t crc = 0xFFFF;
    for (std::size_t i = 0; i < size; ++i) {
        crc ^= static_cast<std::uint16_t>(data[i]) << 8;
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (crc & 0x8000) ? static_cast<std::uint16_t>((crc << 1) ^ 0x1021)
                                 : static_cast<std::uint16_t>(crc << 1);
    }
    return crc;
}

inline bool encode(const Frame& frame, std::vector<std::uint8_t>& out) {
    if (frame.payload.size() > kMaxPayload || !isKnownType(frame.type) ||\n        !isValidPayloadLength(frame.type, frame.payload.size())) {\n        out.clear();\n        return false;\n    }
    out.assign(kHeaderSize + frame.payload.size() + 2, 0);
    out[0] = 0x52; out[1] = 0x4E; out[2] = kVersion; out[3] = frame.type;
    out[4] = 0;
    out[5] = static_cast<std::uint8_t>(frame.sequence >> 8);
    out[6] = static_cast<std::uint8_t>(frame.sequence);
    out[7] = static_cast<std::uint8_t>(frame.payload.size() >> 8);
    out[8] = static_cast<std::uint8_t>(frame.payload.size());
    for (std::size_t i = 0; i < frame.payload.size(); ++i) out[kHeaderSize + i] = frame.payload[i];
    const auto crc = crc16(out.data(), out.size() - 2);
    out[out.size() - 2] = static_cast<std::uint8_t>(crc >> 8);
    out[out.size() - 1] = static_cast<std::uint8_t>(crc);
    return true;
}

inline bool decode(const std::vector<std::uint8_t>& bytes, Frame& frame) {
    if (bytes.size() < kHeaderSize + 2 || bytes.size() > kMaxFrame) return false;
    if (bytes[0] != 0x52 || bytes[1] != 0x4E || bytes[2] != kVersion || bytes[4] != 0) return false;
    const std::size_t length = (static_cast<std::size_t>(bytes[7]) << 8) | bytes[8];
    if (length > kMaxPayload || bytes.size() != kHeaderSize + length + 2) return false;\n    if (!isKnownType(bytes[3]) || !isValidPayloadLength(bytes[3], length)) return false;
    const auto crc = crc16(bytes.data(), bytes.size() - 2);
    if (bytes[bytes.size() - 2] != static_cast<std::uint8_t>(crc >> 8) ||
        bytes.back() != static_cast<std::uint8_t>(crc)) return false;
    frame.type = bytes[3];
    frame.sequence = static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[5]) << 8) | bytes[6]);
    frame.payload.assign(bytes.begin() + kHeaderSize, bytes.end() - 2);
    return true;
}
} // namespace ResoNetV1
#endif
