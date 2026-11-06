#include "resonet_protocol_v1.h"
#include <cassert>
#include <cstdint>
#include <vector>

int main() {
    const std::uint8_t check[] = {'1','2','3','4','5','6','7','8','9'};
    assert(ResoNetV1::crc16(check, sizeof(check)) == 0x29B1);

    ResoNetV1::Frame vector;
    vector.type = ResoNetV1::kHelloReq;
    vector.sequence = 1;
    std::vector<std::uint8_t> vector_bytes;
    assert(ResoNetV1::encode(vector, vector_bytes));
    const std::uint8_t expected[] = {
        0x52, 0x4E, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x70, 0x02
    };
    assert(vector_bytes.size() == sizeof(expected));
    for (std::size_t i = 0; i < sizeof(expected); ++i) assert(vector_bytes[i] == expected[i]);

    ResoNetV1::Frame source;
    source.type = ResoNetV1::kHelloReq;
    source.sequence = 0x1234;
    std::vector<std::uint8_t> encoded;
    assert(ResoNetV1::encode(source, encoded));
    assert(encoded.size() == 11);

    ResoNetV1::Frame decoded;
    assert(ResoNetV1::decode(encoded, decoded));
    assert(decoded.type == source.type);
    assert(decoded.sequence == source.sequence);
    assert(decoded.payload.empty());

    encoded.back() ^= 0x01;
    assert(!ResoNetV1::decode(encoded, decoded));
    encoded.back() ^= 0x01;
    encoded[8] = 0x01;
    assert(!ResoNetV1::decode(encoded, decoded));
    return 0;
}
