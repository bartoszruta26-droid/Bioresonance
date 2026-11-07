package com.resonet.nano

/**
 * Pure presentation model for remote-control readiness.
 *
 * Connection state and control readiness are separate dimensions:
 * a successful TCP/diagnostic connection does not imply actuator control.
 */
data class CapabilityPresentation(
    val statusText: String,
    val controlReady: Boolean
)

fun capabilityPresentation(
    connectionState: ConnectionState,
    capabilities: DeviceCapabilities
): CapabilityPresentation {
    val ready = capabilities.remoteControlReady
    val text = when {
        ready -> "Sterowanie zdalne: gotowość potwierdzona kontraktem urządzenia"
        connectionState is ConnectionState.Connected ->
            "Tryb: DIAGNOSTYKA — sterowanie zdalne niedostępne"
        else ->
            "Sterowanie zdalne: niedostępne (brak zweryfikowanego kontraktu)"
    }
    return CapabilityPresentation(text, ready)
}
