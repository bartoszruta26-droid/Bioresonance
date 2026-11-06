package com.resonet.nano

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.BufferedReader
import java.io.InputStreamReader
import java.net.ServerSocket
import java.nio.charset.StandardCharsets
import kotlin.concurrent.thread
import java.util.concurrent.atomic.AtomicReference

class ArduinoCommunicationTest {
    @Test
    fun commandFrameIsNewlineDelimitedAndResponseIsBounded() = runBlocking {
        ServerSocket(0).use { server ->
            val receivedCommand = AtomicReference("")
            val peer = thread {
                server.accept().use { socket ->
                    receivedCommand.set(BufferedReader(
                        InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8)
                    ).readLine() ?: "")
                    socket.getOutputStream().apply {
                        write("=== System Status ===\nUptime: 12s\n".toByteArray(StandardCharsets.UTF_8))
                        flush()
                    }
                    Thread.sleep(2300)
                }
            }

            val client = ArduinoCommunication()
            try {
                assertTrue(client.connect("127.0.0.1", server.localPort).isSuccess)
                val response = client.sendCommand("s").getOrThrow()
                assertEquals("s", receivedCommand.get())
                assertTrue(response.startsWith("=== System Status ===\n"))
                assertTrue(response.endsWith("\n"))
                assertTrue(response.toByteArray(StandardCharsets.UTF_8).size <= ArduinoCommunication.MAX_RESPONSE_BYTES)
            } finally {
                client.disconnect()
                peer.join(3000)
            }
        }
    }

    @Test
    fun commandInjectionByNewlineIsRejected() = runBlocking {
        val client = ArduinoCommunication()
        val result = client.sendCommand("s\nSTART")
        assertTrue(result.isFailure)
    }
}
