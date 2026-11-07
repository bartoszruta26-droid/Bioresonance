#include "resonet_protocol_v1.h"

#include <cstdint>
#include <iostream>
#include <vector>

namespace {
bool check(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
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
    if (!check(encoded.size() == 11, "round-trip frame length")) return 6;

    ResoNetV1::Frame decoded;
    if (!check(ResoNetV1::decode(encoded, decoded), "decode valid frame")) return 7;
    if (!check(decoded.type == source.type, "decoded frame type")) return 8;
    if (!check(decoded.sequence == source.sequence, "decoded sequence")) return 9;
    if (!check(decoded.payload.empty(), "decoded empty payload")) return 10;

    encoded.back() ^= 0x01;
    if (!check(!ResoNetV1::decode(encoded, decoded), "reject invalid CRC")) return 11;
    encoded.back() ^= 0x01;
    encoded[8] = 0x01;
    if (!check(!ResoNetV1::decode(encoded, decoded), "reject inconsistent payload length")) return 12;

    return 0;
}
