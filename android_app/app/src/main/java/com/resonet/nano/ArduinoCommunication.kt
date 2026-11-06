package com.resonet.nano

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import java.io.IOException
import java.io.InputStream
import java.net.InetSocketAddress
import java.net.Socket
import java.util.concurrent.CopyOnWriteArrayList

/**
 * TCP client for ResoNet Protocol v1.
 *
 * Connection is accepted only after a valid HELLO_RESP. This client exposes
 * diagnostics only; all actuator/configuration operations remain disabled.
 */
class ArduinoCommunication {
    companion object {
        const val DEFAULT_PORT = 5001
        const val CONNECTION_TIMEOUT_MS = 3000
        const val COMMAND_TIMEOUT_MS = 2000
    }

    data class DeviceIdentity(
        val deviceIdHex: String,
        val firmwareVersion: String,
        val capabilities: Int
    )

    private val operationMutex = Mutex()
    @Volatile private var socket: Socket? = null
    @Volatile var connectionState: ConnectionState = ConnectionState.Disconnected
        private set
    @Volatile var deviceIdentity: DeviceIdentity? = null
        private set
    private var nextSequence = 0
    private val listeners = CopyOnWriteArrayList<ConnectionStateListener>()

    interface ConnectionStateListener {
        fun onConnectionStateChanged(state: ConnectionState)
    }

    suspend fun connect(ipAddress: String, port: Int = DEFAULT_PORT): Result<Unit> =
        withContext(Dispatchers.IO) {
            if (ipAddress.isBlank() || port !in 1..65535) {
                return@withContext Result.failure(
                    IllegalArgumentException("Nieprawidłowy adres lub port")
                )
            }

            notifyListeners(ConnectionState.Connecting)
            val result = operationMutex.withLock {
                closeSocketLocked()
                connectionState = ConnectionState.Connecting
                val candidate = Socket()
                try {
                    candidate.tcpNoDelay = true
                    candidate.keepAlive = true
                    candidate.connect(
                        InetSocketAddress(ipAddress, port),
                        CONNECTION_TIMEOUT_MS
                    )
                    candidate.soTimeout = COMMAND_TIMEOUT_MS

                    val sequence = nextSequenceLocked()
                    writeFrame(candidate, ResoNetProtocolV1.Frame(
                        type = ResoNetProtocolV1.TYPE_HELLO_REQ,
                        sequence = sequence
                    ))
                    val response = readFrame(candidate)
                    if (response.type != ResoNetProtocolV1.TYPE_HELLO_RESP ||
                        response.sequence != sequence || response.payload.size != 13
                    ) {
                        throw IOException("Nieprawidłowa odpowiedź HELLO urządzenia")
                    }

                    val payload = response.payload
                    val deviceId = payload.copyOfRange(0, 8)
                        .joinToString("") { "%02X".format(it.toInt() and 0xFF) }
                    val firmware = "${payload[8].toInt() and 0xFF}." +
                        "${payload[9].toInt() and 0xFF}." +
                        "${payload[10].toInt() and 0xFF}"
                    val capabilities = readU16(payload, 11)

                    socket = candidate
                    deviceIdentity = DeviceIdentity(deviceId, firmware, capabilities)
                    connectionState = ConnectionState.Connected(ipAddress, port)
                    Result.success(Unit)
                } catch (error: Exception) {
                    try { candidate.close() } catch (_: IOException) {}
                    socket = null
                    deviceIdentity = null
                    connectionState = ConnectionState.Error(
                        "Błąd połączenia/protokołu: ${error.message ?: error.javaClass.simpleName}"
                    )
                    Result.failure(error)
                }
            }
            notifyListeners(connectionState)
            result
        }

    suspend fun disconnect(): Unit = withContext(Dispatchers.IO) {
        operationMutex.withLock {
            closeSocketLocked()
            connectionState = ConnectionState.Disconnected
        }
        notifyListeners(connectionState)
    }

    suspend fun ping(): Result<Unit> =
        requestFrame(ResoNetProtocolV1.TYPE_PING, ResoNetProtocolV1.TYPE_PONG)
            .map { Unit }

    suspend fun getStatus(): Result<SystemStatus> =
        requestFrame(
            ResoNetProtocolV1.TYPE_STATUS_REQ,
            ResoNetProtocolV1.TYPE_STATUS_RESP
        ).mapCatching { parseStatusFrame(it) }

    /**
     * Text commands are deliberately rejected: v1 is a binary framed protocol.
     */
    suspend fun sendCommand(command: String): Result<String> =
        Result.failure(UnsupportedOperationException(
            "Tekstowe komendy legacy są wyłączone; używany jest binarny protokół ResoNet v1"
        ))

    suspend fun configureChannel(config: ChannelConfig): Result<Unit> =
        Result.failure(UnsupportedOperationException(
            "Konfiguracja kanałów nie jest częścią diagnostycznego protokołu v1"
        ))

    suspend fun startTherapy(): Result<Unit> =
        Result.failure(UnsupportedOperationException(
            "Zdalne uruchamianie wyjść jest wyłączone"
        ))

    suspend fun stopTherapy(): Result<Unit> =
        Result.failure(UnsupportedOperationException(
            "Zdalne sterowanie wyjściami jest wyłączone"
        ))

    suspend fun scanDevices(): Result<EffectorType> =
        Result.failure(UnsupportedOperationException(
            "Skanowanie efektorów nie jest dostępne w protokole diagnostycznym v1"
        ))

    suspend fun getLogs(): Result<List<LogEntry>> =
        Result.failure(UnsupportedOperationException(
            "Zdalne pobieranie logów nie jest dostępne w protokole diagnostycznym v1"
        ))

    private suspend fun requestFrame(requestType: Int, expectedType: Int)
        : Result<ResoNetProtocolV1.Frame> = withContext(Dispatchers.IO) {
        var notifyError = false
        val result = operationMutex.withLock {
            val active = socket
            if (active == null || active.isClosed || !active.isConnected ||
                connectionState !is ConnectionState.Connected
            ) {
                return@withLock Result.failure(
                    IllegalStateException("Brak zweryfikowanego połączenia TCP")
                )
            }

            try {
                val sequence = nextSequenceLocked()
                writeFrame(active, ResoNetProtocolV1.Frame(
                    type = requestType,
                    sequence = sequence
                ))
                val response = readFrame(active)
                if (response.sequence != sequence) {
                    throw IOException("Niezgodny numer sequence odpowiedzi")
                }
                if (response.type == ResoNetProtocolV1.TYPE_NACK ||
                    response.type == ResoNetProtocolV1.TYPE_ERROR
                ) {
                    val code = if (response.payload.size >= 2)
                        response.payload[1].toInt() and 0xFF else -1
                    throw IOException("Urządzenie odrzuciło żądanie (kod $code)")
                }
                if (response.type != expectedType) {
                    throw IOException("Nieoczekiwany typ odpowiedzi: 0x${response.type.toString(16)}")
                }
                val expectedPayloadLength = when (expectedType) {
                    ResoNetProtocolV1.TYPE_PONG -> 0
                    ResoNetProtocolV1.TYPE_STATUS_RESP -> 14
                    else -> throw IOException("Nieobsługiwany typ odpowiedzi diagnostycznej")
                }
                if (response.payload.size != expectedPayloadLength) {
                    throw IOException("Nieprawidłowa długość payloadu odpowiedzi")
                }
                Result.success(response)
            } catch (error: Exception) {
                closeSocketLocked()
                connectionState = ConnectionState.Error(
                    "Błąd transmisji/protokołu: ${error.message ?: error.javaClass.simpleName}"
                )
                notifyError = true
                Result.failure(error)
            }
        }
        if (notifyError) notifyListeners(connectionState)
        result
    }

    private fun writeFrame(active: Socket, frame: ResoNetProtocolV1.Frame) {
        val bytes = ResoNetProtocolV1.encode(frame)
        active.getOutputStream().apply {
            write(bytes)
            flush()
        }
    }

    private fun readFrame(active: Socket): ResoNetProtocolV1.Frame {
        val input = active.getInputStream()
        val header = ByteArray(ResoNetProtocolV1.HEADER_BYTES)
        readFully(input, header)
        if ((header[0].toInt() and 0xFF) != 0x52 ||
            (header[1].toInt() and 0xFF) != 0x4E
        ) throw IOException("Nieprawidłowy znacznik początku ramki")

        val payloadLength = readU16(header, 7)
        if (payloadLength > ResoNetProtocolV1.MAX_PAYLOAD_BYTES) {
            throw IOException("Payload przekracza limit protokołu")
        }

        val tail = ByteArray(payloadLength + ResoNetProtocolV1.CRC_BYTES)
        readFully(input, tail)
        return ResoNetProtocolV1.decode(header + tail)
    }

    private fun readFully(input: InputStream, target: ByteArray) {
        var offset = 0
        while (offset < target.size) {
            val count = input.read(target, offset, target.size - offset)
            if (count < 0) throw IOException("Połączenie zamknięte w środku ramki")
            if (count == 0) continue
            offset += count
        }
    }

    private fun parseStatusFrame(frame: ResoNetProtocolV1.Frame): SystemStatus {
        val payload = frame.payload
        if (payload.size != 14) throw IOException("STATUS_RESP musi mieć 14 bajtów")

        val protocolState = payload[0].toInt() and 0xFF
        val state = when (protocolState) {
            0x00 -> SystemState.BOOT
            0x01 -> SystemState.IDLE
            0x02 -> SystemState.DIAGNOSTIC
            0x03 -> SystemState.BUSY
            0x04 -> SystemState.ERROR
            0x05 -> SystemState.SAFE_LOCKOUT
            else -> throw IOException("Nieznany system_state: $protocolState")
        }

        val faultFlags = readU16(payload, 1)
        if ((faultFlags and 0xFFFC) != 0) throw IOException("Nieznane bity fault_flags")
        val uptime = readU32(payload, 3)
        val freeMemory = readU16(payload, 7)
        val rawTemperature = readI16(payload, 9)
        val sensorFlags = readU16(payload, 11)
        val outputInhibitedRaw = payload[13].toInt() and 0xFF
        if (outputInhibitedRaw !in 0..1) throw IOException("Nieprawidłowe output_inhibited")
        if (sensorFlags != 0) throw IOException("Nieobsługiwane flagi sensor_valid_flags")

        val temperatureValid = rawTemperature != 0x7FFF
        return SystemStatus(
            uptimeSeconds = uptime,
            state = state,
            temperatureCelsius = if (temperatureValid) rawTemperature / 100.0f else 0.0f,
            freeMemoryBytes = freeMemory,
            pwmIsActive = false,
            currentFrequency = 0.0f,
            networkConnected = true,
            detectedEffector = EffectorType.NONE,
            systemStateReported = true,
            pwmStatusKnown = false,
            frequencyKnown = false,
            effectorKnown = false,
            temperatureValid = temperatureValid,
            outputInhibited = if (outputInhibitedRaw == 1) true else null,
            diagnosticFaultFlags = faultFlags
        )
    }

    private fun nextSequenceLocked(): Int {
        val value = nextSequence
        nextSequence = (nextSequence + 1) and 0xFFFF
        return value
    }

    private fun readU16(bytes: ByteArray, offset: Int): Int =
        ((bytes[offset].toInt() and 0xFF) shl 8) or
            (bytes[offset + 1].toInt() and 0xFF)

    private fun readI16(bytes: ByteArray, offset: Int): Int {
        val value = readU16(bytes, offset)
        return if (value and 0x8000 != 0) value - 0x10000 else value
    }

    private fun readU32(bytes: ByteArray, offset: Int): Long =
        ((bytes[offset].toLong() and 0xFF) shl 24) or
            ((bytes[offset + 1].toLong() and 0xFF) shl 16) or
            ((bytes[offset + 2].toLong() and 0xFF) shl 8) or
            (bytes[offset + 3].toLong() and 0xFF)

    private fun closeSocketLocked() {
        val old = socket
        socket = null
        deviceIdentity = null
        try { old?.close() } catch (_: IOException) {}
    }

    /** Synchronous lifecycle cleanup for ViewModel.onCleared(). */
    fun close() {
        closeSocketLocked()
        connectionState = ConnectionState.Disconnected
        notifyListeners(connectionState)
    }

    fun addConnectionListener(listener: ConnectionStateListener) {
        listeners.addIfAbsent(listener)
    }

    fun removeConnectionListener(listener: ConnectionStateListener) {
        listeners.remove(listener)
    }

    private fun notifyListeners(state: ConnectionState) {
        listeners.forEach { listener ->
            try { listener.onConnectionStateChanged(state) } catch (_: Exception) {
                // An observer failure must not interrupt other listeners.
            }
        }
    }
}
