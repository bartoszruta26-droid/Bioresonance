package com.resonet.nano

/**
 * Capabilities reported by a remote device and verified by the client.
 *
 * All control-related fields default to unavailable. A diagnostic connection
 * or a protocol-v1 HELLO response must never enable remote output control.
 *
 * This is a client-side readiness gate, not a substitute for firmware
 * interlocks, independent hardware protection, or safety validation.
 */
data class DeviceCapabilities(
    val remoteControlContractVersion: Int? = null,
    val remoteControlAdvertised: Boolean = false,
    val sessionAuthenticated: Boolean = false,
    val hardwareInterlockVerified: Boolean = false,
    val outputFeedbackVerified: Boolean = false
) {
    /**
     * True only when every prerequisite for the future control transport
     * has been explicitly established. No current protocol-v1 path sets these.
     */
    val remoteControlReady: Boolean
        get() = (remoteControlContractVersion ?: 0) > 0 &&
            remoteControlAdvertised &&
            sessionAuthenticated &&
            hardwareInterlockVerified &&
            outputFeedbackVerified

    companion object {
        /** Safe default for protocol-v1 and unknown devices. */
        val DIAGNOSTIC_ONLY = DeviceCapabilities()
    }
}
