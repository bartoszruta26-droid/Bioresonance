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

static void writeCrc(uint8_t* frame, size_t payload_length) {
    const uint16_t crc = crc16(frame, HEADER_SIZE + payload_length);
    frame[HEADER_SIZE + payload_length] = (uint8_t)(crc >> 8);
    frame[HEADER_SIZE + payload_length + 1] = (uint8_t)crc;
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
    writeCrc(bad_flags, 0);
    handled = 0;
    parser.reset();
    feedAll(parser, bad_flags, sizeof(bad_flags));
    assert(handled == 0 && parser.rejectedFrames() == 2);

    assert(encode(0x55, 1, NULL, 0, encoded, sizeof(encoded)) == 0);
    assert(encode(HELLO_REQ, 1, expected, 1, encoded, sizeof(encoded)) == 0);

    // Unknown types and response-only types must never enter the request handler.
    uint8_t invalid_type[sizeof(expected)];
    memcpy(invalid_type, expected, sizeof(expected));
    invalid_type[3] = 0x55;
    writeCrc(invalid_type, 0);
    handled = 0;
    parser.reset();
    feedAll(parser, invalid_type, sizeof(invalid_type));
    assert(handled == 0 && parser.rejectedFrames() == 3);

    uint8_t response_as_request[sizeof(expected)];
    memcpy(response_as_request, expected, sizeof(expected));
    response_as_request[3] = HELLO_RESP;
    writeCrc(response_as_request, 0);
    parser.reset();
    feedAll(parser, response_as_request, sizeof(response_as_request));
    assert(handled == 0 && parser.rejectedFrames() == 4);

    // A valid request immediately after a rejected header is still recognized.
    uint8_t header_then_valid[sizeof(invalid_type) + sizeof(expected)];
    memcpy(header_then_valid, invalid_type, sizeof(invalid_type));
    memcpy(header_then_valid + sizeof(invalid_type), expected, sizeof(expected));
    handled = 0;
    parser.reset();
    feedAll(parser, header_then_valid, sizeof(header_then_valid));
    assert(handled == 1 && last_type == HELLO_REQ && last_sequence == 1);

    // Preserve a trailing 0x52 across a bad CRC so the next TCP read can
    // complete the split magic sequence 0x52, 0x4E.
    uint8_t bad_crc_prefix[sizeof(expected)];
    memcpy(bad_crc_prefix, expected, sizeof(expected));
    bad_crc_prefix[10] = MAGIC_0;
    uint8_t split_recovery[sizeof(bad_crc_prefix) + sizeof(expected) - 1];
    memcpy(split_recovery, bad_crc_prefix, sizeof(bad_crc_prefix));
    memcpy(split_recovery + sizeof(bad_crc_prefix), expected + 1, sizeof(expected) - 1);
    handled = 0;
    parser.reset();
    feedAll(parser, split_recovery, sizeof(split_recovery));
    assert(handled == 1 && last_type == HELLO_REQ && last_sequence == 1);
    assert(!parser.hasPartialFrame());

    parser.reset();
    feedAll(parser, expected, 5);
    assert(parser.hasPartialFrame());
    parser.reset();
    assert(!parser.hasPartialFrame());
    return 0;
}
