#include "../../ResoNet_Diagnostic_Nano/diagnostic_protocol.h"
#include <cassert>
#include <cstdint>
#include <vector>

using namespace ResoNetDiagnostic;

static std::vector<uint8_t> request(uint8_t type, uint16_t sequence) {
    uint8_t frame[11];
    const size_t size = encode(type, sequence, 0, 0, frame, sizeof(frame));
    assert(size == sizeof(frame));
    return std::vector<uint8_t>(frame, frame + size);
}

int main() {
    const uint8_t check[] = {'1','2','3','4','5','6','7','8','9'};
    assert(Parser::crc16(check, sizeof(check)) == 0x29B1);

    const std::vector<uint8_t> canonical = request(HELLO_REQ, 1);
    const uint8_t expected[] = {
        0x52, 0x4E, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x70, 0x02
    };
    assert(canonical.size() == sizeof(expected));
    for (size_t i = 0; i < sizeof(expected); ++i) assert(canonical[i] == expected[i]);

    Parser parser;
    Request decoded = {};
    for (size_t i = 0; i + 1 < canonical.size(); ++i)
        assert(!parser.feed(canonical[i], decoded));
    assert(parser.feed(canonical.back(), decoded));
    assert(decoded.type == HELLO_REQ && decoded.sequence == 1);

    std::vector<uint8_t> ping = request(PING, 0x1234);
    bool ready = false;
    for (size_t i = 0; i < ping.size(); ++i) ready = parser.feed(ping[i], decoded) || ready;
    assert(ready && decoded.type == PING && decoded.sequence == 0x1234);

    ping[10] ^= 0x01;
    for (size_t i = 0; i < ping.size(); ++i)
        assert(!parser.feed(ping[i], decoded));

    std::vector<uint8_t> bad_flags = request(STATUS_REQ, 7);
    bad_flags[4] = 1;
    const uint16_t crc = Parser::crc16(&bad_flags[0], 9);
    bad_flags[9] = static_cast<uint8_t>(crc >> 8);
    bad_flags[10] = static_cast<uint8_t>(crc);
    for (size_t i = 0; i < bad_flags.size(); ++i)
        assert(!parser.feed(bad_flags[i], decoded));

    std::vector<uint8_t> bad_length = request(STATUS_REQ, 8);
    bad_length[8] = 1;
    for (size_t i = 0; i < bad_length.size(); ++i)
        assert(!parser.feed(bad_length[i], decoded));

    return 0;
}
