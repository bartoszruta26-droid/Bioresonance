package com.resonet.nano

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertTrue
import org.junit.Test

class ArduinoCommunicationTest {
    @Test
    fun legacyTextCommandsAreRejected() = runBlocking {
        val client = ArduinoCommunication()
        val result = client.sendCommand("s")
        assertTrue(result.isFailure)
        assertTrue(result.exceptionOrNull() is UnsupportedOperationException)
    }

    @Test
    fun actuatorAndConfigurationOperationsRemainDisabled() = runBlocking {
        val client = ArduinoCommunication()
        assertUnsupported(client.configureChannel(ChannelConfig(id = 1)))
        assertUnsupported(client.startTherapy())
        assertUnsupported(client.stopTherapy())
        assertUnsupported(client.scanDevices())
        assertUnsupported(client.getLogs())
    }

    private fun assertUnsupported(result: Result<*>) {
        assertTrue(result.isFailure)
        assertTrue(result.exceptionOrNull() is UnsupportedOperationException)
    }
}
