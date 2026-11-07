package com.resonet.nano

import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class DeviceCapabilitiesTest {
    @Test
    fun defaultsToDiagnosticOnly() {
        assertFalse(DeviceCapabilities().remoteControlReady)
        assertFalse(DeviceCapabilities.DIAGNOSTIC_ONLY.remoteControlReady)
    }

    @Test
    fun everyControlPrerequisiteIsMandatory() {
        val complete = DeviceCapabilities(
            remoteControlContractVersion = 1,
            remoteControlAdvertised = true,
            sessionAuthenticated = true,
            hardwareInterlockVerified = true,
            outputFeedbackVerified = true
        )

        assertTrue(complete.remoteControlReady)
        assertFalse(complete.copy(remoteControlAdvertised = false).remoteControlReady)
        assertFalse(complete.copy(sessionAuthenticated = false).remoteControlReady)
        assertFalse(complete.copy(hardwareInterlockVerified = false).remoteControlReady)
        assertFalse(complete.copy(outputFeedbackVerified = false).remoteControlReady)
        assertFalse(complete.copy(remoteControlContractVersion = null).remoteControlReady)
        assertFalse(complete.copy(remoteControlContractVersion = 0).remoteControlReady)
    }

    @Test
    fun partialHandshakeCannotEnableRemoteControl() {
        val diagnosticHandshake = DeviceCapabilities(
            remoteControlAdvertised = true,
            remoteControlContractVersion = 1
        )

        assertFalse(diagnosticHandshake.remoteControlReady)
    }
}
