#include "protocol_v1.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

using namespace ResoNetProtocolV1;

static unsigned int handled = 0;
static uint8_t last_type = 0;
static uint16_t last_sequence = 0;

static void onFrame(const FrameView& frame, void*) {
    ++handled;
    last_type = frame.type;
    last_sequence = frame.sequence;
}

static void feedAll(Parser& parser, const uint8_t* bytes, size_t length) {
    for (size_t i = 0; i < length; ++i) parser.feed(bytes[i], onFrame, NULL);
}

int main() {
    const uint8_t expected[] = {
        0x52, 0x4E, 0x01, 0x01, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x70, 0x02
    };
    uint8_t encoded[MAX_FRAME_SIZE];
    const size_t encoded_length = encode(HELLO_REQ, 1, NULL, 0, encoded, sizeof(encoded));
    assert(encoded_length == sizeof(expected));
    assert(memcmp(encoded, expected, sizeof(expected)) == 0);

    Parser parser;
    handled = 0;
    feedAll(parser, expected, sizeof(expected));
    assert(handled == 1 && last_type == HELLO_REQ && last_sequence == 1);
    assert(!parser.hasPartialFrame());

    uint8_t ping[MAX_FRAME_SIZE];
    const size_t ping_length = encode(PING, 7, NULL, 0, ping, sizeof(ping));
    assert(ping_length == 11);
    uint8_t combined[22];
    memcpy(combined, expected, sizeof(expected));
    memcpy(combined + sizeof(expected), ping, ping_length);
    handled = 0;
    parser.reset();
    feedAll(parser, combined, sizeof(combined));
    assert(handled == 2 && last_type == PING && last_sequence == 7);

    uint8_t corrupt[sizeof(expected)];
    memcpy(corrupt, expected, sizeof(expected));
    corrupt[10] ^= 0x01;
    handled = 0;
    parser.reset();
    feedAll(parser, corrupt, sizeof(corrupt));
    assert(handled == 0 && parser.rejectedFrames() == 1);

    uint8_t bad_flags[sizeof(expected)];
    memcpy(bad_flags, expected, sizeof(expected));
    bad_flags[4] = 0x01;
    handled = 0;
    parser.reset();
    feedAll(parser, bad_flags, sizeof(bad_flags));
    assert(handled == 0 && parser.rejectedFrames() == 2);

    assert(encode(0x55, 1, NULL, 0, encoded, sizeof(encoded)) == 0);
    assert(encode(HELLO_REQ, 1, expected, 1, encoded, sizeof(encoded)) == 0);

    parser.reset();
    feedAll(parser, expected, 5);
    assert(parser.hasPartialFrame());
    parser.reset();
    assert(!parser.hasPartialFrame());
    return 0;
}
