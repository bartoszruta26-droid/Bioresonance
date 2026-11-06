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
    fun actuatorOperationsRemainDisabled() = runBlocking {
        val client = ArduinoCommunication()
        assertTrue(client.startTherapy().isFailure)
        assertTrue(client.stopTherapy().isFailure)
    }
}
