package com.resonet.nano

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class CapabilityPresentationTest {
    private val diagnosticOnly = DeviceCapabilities.DIAGNOSTIC_ONLY

    @Test
    fun disconnectedStateShowsControlUnavailable() {
        val result = capabilityPresentation(ConnectionState.Disconnected, diagnosticOnly)

        assertEquals(
            "Sterowanie zdalne: niedostępne (brak zweryfikowanego kontraktu)",
            result.statusText
        )
        assertFalse(result.controlReady)
    }

    @Test
    fun connectingStateDoesNotImplyControlReadiness() {
        val result = capabilityPresentation(ConnectionState.Connecting, diagnosticOnly)

        assertEquals(
            "Sterowanie zdalne: niedostępne (brak zweryfikowanego kontraktu)",
            result.statusText
        )
        assertFalse(result.controlReady)
    }

    @Test
    fun connectedDiagnosticDeviceIsStillReadOnly() {
        val result = capabilityPresentation(
            ConnectionState.Connected("192.0.2.10", 5001),
            diagnosticOnly
        )

        assertEquals(
            "Tryb: DIAGNOSTYKA — sterowanie zdalne niedostępne",
            result.statusText
        )
        assertFalse(result.controlReady)
    }

    @Test
    fun errorStateDoesNotExposeControl() {
        val result = capabilityPresentation(ConnectionState.Error("timeout"), diagnosticOnly)

        assertEquals(
            "Sterowanie zdalne: niedostępne (brak zweryfikowanego kontraktu)",
            result.statusText
        )
        assertFalse(result.controlReady)
    }

    @Test
    fun readyPresentationRequiresCompleteVerifiedCapabilities() {
        val complete = DeviceCapabilities(
            remoteControlContractVersion = 1,
            remoteControlAdvertised = true,
            sessionAuthenticated = true,
            hardwareInterlockVerified = true,
            outputFeedbackVerified = true
        )
        val result = capabilityPresentation(ConnectionState.Connected("192.0.2.10", 5001), complete)

        assertTrue(result.controlReady)
        assertEquals(
            "Sterowanie zdalne: gotowość potwierdzona kontraktem urządzenia",
            result.statusText
        )
    }
}
