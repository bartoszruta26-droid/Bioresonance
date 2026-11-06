package com.resonet.nano

/**
 * Diagnostic-only ResoNet Protocol v1 frame codec.
 *
 * This codec does not expose actuator commands. ArduinoCommunication uses it
 * for the diagnostic TCP handshake and status exchange. Other client platforms
 * still require their own protocol v1 adapters and conformance checks.
 */
object ResoNetProtocolV1 {
    const val VERSION = 1
    const val MAX_PAYLOAD_BYTES = 192
    const val HEADER_BYTES = 9
    const val CRC_BYTES = 2
    const val MAX_FRAME_BYTES = HEADER_BYTES + MAX_PAYLOAD_BYTES + CRC_BYTES

    const val TYPE_HELLO_REQ = 0x01
    const val TYPE_HELLO_RESP = 0x02
    const val TYPE_PING = 0x03
    const val TYPE_PONG = 0x04
    const val TYPE_STATUS_REQ = 0x05
    const val TYPE_STATUS_RESP = 0x06
    const val TYPE_NACK = 0x7D
    const val TYPE_ERROR = 0x7E
    const val TYPE_ACK = 0x7F

    data class Frame(
        val type: Int,
        val sequence: Int,
        val payload: ByteArray = byteArrayOf(),
        val flags: Int = 0
    )

    fun encode(frame: Frame): ByteArray {
        require(frame.type in 0..255) { "type poza zakresem bajtu" }
        require(frame.sequence in 0..65535) { "sequence poza zakresem uint16" }
        require(frame.flags == 0) { "flags muszą być równe zero w protokole v1" }
        require(frame.payload.size <= MAX_PAYLOAD_BYTES) { "payload przekracza limit" }

        val output = ByteArray(HEADER_BYTES + frame.payload.size + CRC_BYTES)
        output[0] = 0x52
        output[1] = 0x4E
        output[2] = VERSION.toByte()
        output[3] = frame.type.toByte()
        output[4] = frame.flags.toByte()
        writeU16(output, 5, frame.sequence)
        writeU16(output, 7, frame.payload.size)
        frame.payload.copyInto(output, HEADER_BYTES)
        val crc = crc16CcittFalse(output, 0, HEADER_BYTES + frame.payload.size)
        writeU16(output, HEADER_BYTES + frame.payload.size, crc)
        return output
    }

    fun decode(bytes: ByteArray): Frame {
        require(bytes.size >= HEADER_BYTES + CRC_BYTES) { "ramka zbyt krótka" }
        require(bytes.size <= MAX_FRAME_BYTES) { "ramka przekracza limit" }
        require(u8(bytes[0]) == 0x52 && u8(bytes[1]) == 0x4E) { "nieprawidłowy magic" }
        require(u8(bytes[2]) == VERSION) { "nieobsługiwana wersja protokołu" }
        require(u8(bytes[4]) == 0) { "nieobsługiwane flags" }

        val payloadLength = readU16(bytes, 7)
        require(payloadLength <= MAX_PAYLOAD_BYTES) { "payload przekracza limit" }
        require(bytes.size == HEADER_BYTES + payloadLength + CRC_BYTES) {
            "długość ramki nie zgadza się z payload_length"
        }

        val expectedCrc = readU16(bytes, HEADER_BYTES + payloadLength)
        val actualCrc = crc16CcittFalse(bytes, 0, HEADER_BYTES + payloadLength)
        require(expectedCrc == actualCrc) { "niezgodne CRC-16" }

        return Frame(
            type = u8(bytes[3]),
            sequence = readU16(bytes, 5),
            payload = bytes.copyOfRange(HEADER_BYTES, HEADER_BYTES + payloadLength),
            flags = u8(bytes[4])
        )
    }

    fun crc16CcittFalse(bytes: ByteArray, offset: Int = 0, length: Int = bytes.size): Int {
        require(offset >= 0 && length >= 0 && offset <= bytes.size - length) {
            "nieprawidłowy zakres CRC"
        }
        var crc = 0xFFFF
        for (index in offset until offset + length) {
            crc = crc xor (u8(bytes[index]) shl 8)
            repeat(8) {
                crc = if ((crc and 0x8000) != 0) {
                    ((crc shl 1) xor 0x1021) and 0xFFFF
                } else {
                    (crc shl 1) and 0xFFFF
                }
            }
        }
        return crc
    }

    private fun u8(value: Byte): Int = value.toInt() and 0xFF

    private fun writeU16(target: ByteArray, offset: Int, value: Int) {
        target[offset] = ((value ushr 8) and 0xFF).toByte()
        target[offset + 1] = (value and 0xFF).toByte()
    }

    private fun readU16(source: ByteArray, offset: Int): Int =
        (u8(source[offset]) shl 8) or u8(source[offset + 1])
}
