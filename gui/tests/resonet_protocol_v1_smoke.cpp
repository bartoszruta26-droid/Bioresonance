#include "resonet_protocol_v1.h"

#include <cstdint>
#include <iostream>
#include <vector>

namespace {
bool check(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}

void refreshCrc(std::vector<std::uint8_t>& bytes) {
    const auto crc = ResoNetV1::crc16(bytes.data(), bytes.size() - 2);
    bytes[bytes.size() - 2] = static_cast<std::uint8_t>(crc >> 8);
    bytes.back() = static_cast<std::uint8_t>(crc);
}
}

int main() {
    const std::uint8_t crc_input[] = {'1','2','3','4','5','6','7','8','9'};
    if (!check(ResoNetV1::crc16(crc_input, sizeof(crc_input)) == 0x29B1,
               "CRC-16/CCITT-FALSE known vector")) return 1;

    ResoNetV1::Frame vector;
    vector.type = ResoNetV1::kHelloReq;
    vector.sequence = 1;
    std::vector<std::uint8_t> vector_bytes;
    if (!check(ResoNetV1::encode(vector, vector_bytes), "encode HELLO vector")) return 2;

    const std::uint8_t expected[] = {
        0x52, 0x4E, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x70, 0x02
    };
    if (!check(vector_bytes.size() == sizeof(expected), "HELLO vector frame length")) return 3;
    for (std::size_t i = 0; i < sizeof(expected); ++i) {
        if (!check(vector_bytes[i] == expected[i], "HELLO vector byte mismatch")) return 4;
    }

    ResoNetV1::Frame source;
    source.type = ResoNetV1::kHelloReq;
    source.sequence = 0x1234;
    std::vector<std::uint8_t> encoded;
    if (!check(ResoNetV1::encode(source, encoded), "encode round-trip frame")) return 5;
    ResoNetV1::Frame decoded;
    if (!check(ResoNetV1::decode(encoded, decoded), "decode valid frame")) return 6;
    if (!check(decoded.type == source.type && decoded.sequence == source.sequence &&
               decoded.payload.empty(), "round-trip fields")) return 7;

    const struct { std::uint8_t type; std::size_t length; } valid_types[] = {
        {ResoNetV1::kHelloReq, 0}, {ResoNetV1::kHelloResp, 13},
        {ResoNetV1::kPing, 0}, {ResoNetV1::kPong, 0},
        {ResoNetV1::kStatusReq, 0}, {ResoNetV1::kStatusResp, 14},
        {ResoNetV1::kNack, 2}, {ResoNetV1::kError, 2}, {ResoNetV1::kAck, 1}
    };
    for (const auto& item : valid_types) {
        ResoNetV1::Frame candidate;
        candidate.type = item.type;
        candidate.payload.assign(item.length, 0);
        std::vector<std::uint8_t> bytes;
        if (!check(ResoNetV1::encode(candidate, bytes), "encode valid type schema")) return 8;
        ResoNetV1::Frame parsed;
        if (!check(ResoNetV1::decode(bytes, parsed), "decode valid type schema")) return 9;
    }

    ResoNetV1::Frame invalid;
    invalid.type = 0x55;
    if (!check(!ResoNetV1::encode(invalid, encoded) && encoded.empty(),
               "reject unknown type on encode")) return 10;
    invalid.type = ResoNetV1::kHelloResp;
    invalid.payload.assign(12, 0);
    if (!check(!ResoNetV1::encode(invalid, encoded) && encoded.empty(),
               "reject invalid payload length on encode")) return 11;
    invalid.type = ResoNetV1::kAck;
    invalid.payload.assign(2, 0);
    if (!check(!ResoNetV1::encode(invalid, encoded),
               "reject invalid ACK payload length")) return 12;

    if (!check(ResoNetV1::encode(source, encoded), "prepare unknown-type frame")) return 13;
    encoded[3] = 0x55;
    refreshCrc(encoded);
    if (!check(!ResoNetV1::decode(encoded, decoded), "reject unknown type on decode")) return 14;

    if (!check(ResoNetV1::encode(source, encoded), "prepare wrong-length frame")) return 15;
    encoded[3] = ResoNetV1::kHelloResp;
    refreshCrc(encoded);
    if (!check(!ResoNetV1::decode(encoded, decoded),
               "reject known type with invalid payload length on decode")) return 16;

    if (!check(ResoNetV1::encode(source, encoded), "prepare bad CRC frame")) return 17;
    encoded.back() ^= 0x01;
    if (!check(!ResoNetV1::decode(encoded, decoded), "reject invalid CRC")) return 18;

    if (!check(ResoNetV1::encode(source, encoded), "prepare inconsistent length frame")) return 19;
    encoded[8] = 0x01;
    if (!check(!ResoNetV1::decode(encoded, decoded),
               "reject inconsistent payload length")) return 20;
    return 0;
}
