package com.resonet.nano

import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Test

class ResoNetProtocolV1Test {
    @Test
    fun helloRequestMatchesPublishedReferenceVector() {
        val encoded = ResoNetProtocolV1.encode(
            ResoNetProtocolV1.Frame(type = ResoNetProtocolV1.TYPE_HELLO_REQ, sequence = 1)
        )
        assertArrayEquals(
            byteArrayOf(
                0x52, 0x4E, 0x01, 0x01, 0x00, 0x00, 0x01,
                0x00, 0x00, 0x70, 0x02
            ),
            encoded
        )
    }

    @Test
    fun decodesValidFrameAndPreservesPayload() {
        val payload = byteArrayOf(
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x20, 0x7F, 0xFF.toByte(), 0x00, 0x00, 0x00
        )
        val encoded = ResoNetProtocolV1.encode(
            ResoNetProtocolV1.Frame(type = ResoNetProtocolV1.TYPE_STATUS_RESP, sequence = 65535, payload = payload)
        )
        val decoded = ResoNetProtocolV1.decode(encoded)
        assertEquals(ResoNetProtocolV1.TYPE_STATUS_RESP, decoded.type)
        assertEquals(65535, decoded.sequence)
        assertArrayEquals(payload, decoded.payload)
    }

    @Test
    fun rejectsBadCrc() {
        val encoded = ResoNetProtocolV1.encode(
            ResoNetProtocolV1.Frame(type = ResoNetProtocolV1.TYPE_PING, sequence = 2)
        )
        encoded[3] = ResoNetProtocolV1.TYPE_STATUS_REQ.toByte()
        assertThrows(IllegalArgumentException::class.java) {
            ResoNetProtocolV1.decode(encoded)
        }
    }

    @Test
    fun rejectsOversizedPayloadBeforeEncoding() {
        assertThrows(IllegalArgumentException::class.java) {
            ResoNetProtocolV1.encode(
                ResoNetProtocolV1.Frame(
                    type = ResoNetProtocolV1.TYPE_PING,
                    sequence = 1,
                    payload = ByteArray(ResoNetProtocolV1.MAX_PAYLOAD_BYTES + 1)
                )
            )
        }
    }

    @Test
    fun rejectsTrailingBytes() {
        val encoded = ResoNetProtocolV1.encode(
            ResoNetProtocolV1.Frame(type = ResoNetProtocolV1.TYPE_PING, sequence = 2)
        ) + byteArrayOf(0x00)
        assertThrows(IllegalArgumentException::class.java) {
            ResoNetProtocolV1.decode(encoded)
        }
    }

    @Test
    fun rejectsUnknownMessageTypeBeforeEncoding() {
        assertThrows(IllegalArgumentException::class.java) {
            ResoNetProtocolV1.encode(
                ResoNetProtocolV1.Frame(type = 0x20, sequence = 1)
            )
        }
    }

    @Test
    fun rejectsKnownMessageTypeWithInvalidPayloadLength() {
        assertThrows(IllegalArgumentException::class.java) {
            ResoNetProtocolV1.encode(
                ResoNetProtocolV1.Frame(
                    type = ResoNetProtocolV1.TYPE_STATUS_RESP,
                    sequence = 1,
                    payload = byteArrayOf(0x01)
                )
            )
        }
    }

    @Test
    fun decoderRejectsUnknownMessageTypeEvenWithValidCrc() {
        val encoded = ResoNetProtocolV1.encode(
            ResoNetProtocolV1.Frame(type = ResoNetProtocolV1.TYPE_PING, sequence = 2)
        )
        encoded[3] = 0x20
        val crc = ResoNetProtocolV1.crc16CcittFalse(encoded, 0, encoded.size - 2)
        encoded[encoded.size - 2] = (crc ushr 8).toByte()
        encoded[encoded.size - 1] = crc.toByte()
        assertThrows(IllegalArgumentException::class.java) {
            ResoNetProtocolV1.decode(encoded)
        }
    }

    @Test
    fun decoderRejectsPayloadLengthThatDoesNotMatchMessageType() {
        val encoded = ResoNetProtocolV1.encode(
            ResoNetProtocolV1.Frame(type = ResoNetProtocolV1.TYPE_PING, sequence = 2)
        )
        encoded[3] = ResoNetProtocolV1.TYPE_STATUS_RESP.toByte()
        val crc = ResoNetProtocolV1.crc16CcittFalse(encoded, 0, encoded.size - 2)
        encoded[encoded.size - 2] = (crc ushr 8).toByte()
        encoded[encoded.size - 1] = crc.toByte()
        assertThrows(IllegalArgumentException::class.java) {
            ResoNetProtocolV1.decode(encoded)
        }
    }

    @Test
    fun rejectsUnsupportedFlags() {
        val encoded = ResoNetProtocolV1.encode(
            ResoNetProtocolV1.Frame(type = ResoNetProtocolV1.TYPE_PING, sequence = 2)
        )
        encoded[4] = 1
        assertThrows(IllegalArgumentException::class.java) {
            ResoNetProtocolV1.decode(encoded)
        }
    }
}
