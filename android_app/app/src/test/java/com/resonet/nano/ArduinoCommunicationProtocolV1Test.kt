package com.resonet.nano

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.DataInputStream
import java.net.ServerSocket
import java.net.Socket
import java.util.concurrent.atomic.AtomicReference

class ArduinoCommunicationProtocolV1Test {
    @Test
    fun connectsOnlyAfterHelloAndReadsDiagnosticStatus() = runBlocking {
        val server = ServerSocket(0)
        val serverFailure = AtomicReference<Throwable?>(null)
        val worker = Thread {
            try {
                server.accept().use { peer ->
                    peer.soTimeout = 3000
                    val hello = readFrame(peer)
                    assertEquals(ResoNetProtocolV1.TYPE_HELLO_REQ, hello.type)
                    assertEquals(0, hello.sequence)

                    val helloPayload = byteArrayOf(
                        'R'.code.toByte(), 'N'.code.toByte(),
                        0, 0, 0, 0, 0, 1,
                        4, 0, 0, 0, 0
                    )
                    writeFrame(peer, ResoNetProtocolV1.Frame(
                        ResoNetProtocolV1.TYPE_HELLO_RESP, hello.sequence, helloPayload
                    ))

                    val statusRequest = readFrame(peer)
                    assertEquals(ResoNetProtocolV1.TYPE_STATUS_REQ, statusRequest.type)
                    assertEquals(1, statusRequest.sequence)

                    val statusPayload = byteArrayOf(
                        0x01,             // IDLE
                        0x00, 0x00,       // fault_flags
                        0x00, 0x00, 0x04, 0xD2.toByte(), // uptime = 1234 s
                        0x06, 0x00,       // free memory = 1536 bytes
                        0x7F, 0xFF.toByte(), // no verified temperature
                        0x00, 0x00,       // no valid sensors
                        0x00              // physical output inhibition unknown
                    )
                    writeFrame(peer, ResoNetProtocolV1.Frame(
                        ResoNetProtocolV1.TYPE_STATUS_RESP,
                        statusRequest.sequence,
                        statusPayload
                    ))
                }
            } catch (error: Throwable) {
                serverFailure.set(error)
            }
        }
        worker.start()

        val communication = ArduinoCommunication()
        try {
            val connected = communication.connect("127.0.0.1", server.localPort)
            assertTrue(connected.isSuccess)
            assertEquals("524E000000000001", communication.deviceIdentity?.deviceIdHex)
            assertEquals("4.0.0", communication.deviceIdentity?.firmwareVersion)

            val status = communication.getStatus().getOrThrow()
            assertEquals(1234L, status.uptimeSeconds)
            assertEquals(1536, status.freeMemoryBytes)
            assertFalse(status.temperatureValid)
            assertNull(status.outputInhibited)
            assertEquals(SystemState.IDLE, status.state)
        } finally {
            communication.disconnect()
            server.close()
            worker.join(3000)
        }
        assertNull(serverFailure.get())
    }

    @Test
    fun nackDuringStatusRequestMovesClientToErrorAndClearsIdentity() = runBlocking {
        val server = ServerSocket(0)
        val serverFailure = AtomicReference<Throwable?>(null)
        val worker = Thread {
            try {
                server.accept().use { peer ->
                    peer.soTimeout = 3000
                    val hello = readFrame(peer)
                    assertEquals(ResoNetProtocolV1.TYPE_HELLO_REQ, hello.type)
                    writeFrame(peer, ResoNetProtocolV1.Frame(
                        ResoNetProtocolV1.TYPE_HELLO_RESP,
                        hello.sequence,
                        byteArrayOf(
                            'R'.code.toByte(), 'N'.code.toByte(),
                            0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0
                        )
                    ))

                    val statusRequest = readFrame(peer)
                    assertEquals(ResoNetProtocolV1.TYPE_STATUS_REQ, statusRequest.type)
                    writeFrame(peer, ResoNetProtocolV1.Frame(
                        ResoNetProtocolV1.TYPE_NACK,
                        statusRequest.sequence,
                        byteArrayOf(ResoNetProtocolV1.TYPE_STATUS_REQ.toByte(), 0x02)
                    ))
                }
            } catch (error: Throwable) {
                serverFailure.set(error)
            }
        }
        worker.start()

        val communication = ArduinoCommunication()
        try {
            assertTrue(communication.connect("127.0.0.1", server.localPort).isSuccess)
            assertTrue(communication.deviceIdentity != null)

            val result = communication.getStatus()
            assertTrue(result.isFailure)
            assertTrue(communication.connectionState is ConnectionState.Error)
            assertNull(communication.deviceIdentity)
        } finally {
            communication.disconnect()
            server.close()
            worker.join(3000)
        }
        assertNull(serverFailure.get())
    }

    @Test
    fun truncatedResponseClosesDiagnosticConnection() = runBlocking {
        val server = ServerSocket(0)
        val serverFailure = AtomicReference<Throwable?>(null)
        val worker = Thread {
            try {
                server.accept().use { peer ->
                    peer.soTimeout = 3000
                    val hello = readFrame(peer)
                    writeFrame(peer, ResoNetProtocolV1.Frame(
                        ResoNetProtocolV1.TYPE_HELLO_RESP,
                        hello.sequence,
                        byteArrayOf(
                            'R'.code.toByte(), 'N'.code.toByte(),
                            0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0
                        )
                    ))
                    val statusRequest = readFrame(peer)
                    assertEquals(ResoNetProtocolV1.TYPE_STATUS_REQ, statusRequest.type)
                    // Send only a partial response header, then close the socket.
                    peer.getOutputStream().write(byteArrayOf(0x52, 0x4E, 0x01))
                    peer.getOutputStream().flush()
                }
            } catch (error: Throwable) {
                serverFailure.set(error)
            }
        }
        worker.start()

        val communication = ArduinoCommunication()
        try {
            assertTrue(communication.connect("127.0.0.1", server.localPort).isSuccess)
            val result = communication.getStatus()
            assertTrue(result.isFailure)
            assertTrue(communication.connectionState is ConnectionState.Error)
            assertNull(communication.deviceIdentity)
        } finally {
            communication.disconnect()
            server.close()
            worker.join(3000)
        }
        assertNull(serverFailure.get())
    }

    private fun readFrame(socket: Socket): ResoNetProtocolV1.Frame {
        val input = DataInputStream(socket.getInputStream())
        val header = ByteArray(ResoNetProtocolV1.HEADER_BYTES)
        input.readFully(header)
        val payloadLength =
            ((header[7].toInt() and 0xFF) shl 8) or (header[8].toInt() and 0xFF)
        val tail = ByteArray(payloadLength + ResoNetProtocolV1.CRC_BYTES)
        input.readFully(tail)
        return ResoNetProtocolV1.decode(header + tail)
    }

    private fun writeFrame(socket: Socket, frame: ResoNetProtocolV1.Frame) {
        socket.getOutputStream().apply {
            write(ResoNetProtocolV1.encode(frame))
            flush()
        }
    }
}
