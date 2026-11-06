package com.resonet.nano

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import java.io.ByteArrayOutputStream
import java.io.IOException
import java.net.InetSocketAddress
import java.net.Socket
import java.net.SocketTimeoutException
import java.nio.ByteBuffer
import java.nio.charset.CodingErrorAction
import java.nio.charset.StandardCharsets
import java.util.concurrent.CopyOnWriteArrayList

/**
 * Klient TCP dla historycznego tekstowego interfejsu diagnostycznego.
 * Firmware nie implementuje obecnie rzeczywistego serwera TCP.
 * Wielowierszowa odpowiedź legacy jest ograniczona rozmiarem i czasem bezczynności;
 * docelowy protokół musi mieć jawny znacznik końca ramki.
 */
class ArduinoCommunication {
    companion object {
        const val DEFAULT_PORT = 5001
        const val CONNECTION_TIMEOUT_MS = 3000
        const val COMMAND_TIMEOUT_MS = 2000
        const val MAX_RESPONSE_BYTES = 8192
        const val MAX_COMMAND_LENGTH = 256
        const val CMD_STATUS = "s"
        const val CMD_START_THERAPY = "START"
        const val CMD_STOP_THERAPY = "STOP"
        const val CMD_LOGS = "l"
        const val CMD_DEVICE_SCAN = "d"
    }

    private val operationMutex = Mutex()
    @Volatile private var socket: Socket? = null
    @Volatile var connectionState: ConnectionState = ConnectionState.Disconnected
        private set
    private val listeners = CopyOnWriteArrayList<ConnectionStateListener>()

    interface ConnectionStateListener {
        fun onConnectionStateChanged(state: ConnectionState)
    }

    suspend fun connect(ipAddress: String, port: Int = DEFAULT_PORT): Result<Unit> =
        withContext(Dispatchers.IO) {
            if (ipAddress.isBlank() || port !in 1..65535) {
                return@withContext Result.failure(IllegalArgumentException("Nieprawidłowy adres lub port"))
            }
            notifyListeners(ConnectionState.Connecting)
            val result = operationMutex.withLock {
                closeSocketLocked()
                connectionState = ConnectionState.Connecting
                val candidate = Socket()
                try {
                    candidate.tcpNoDelay = true
                    candidate.keepAlive = true
                    candidate.connect(InetSocketAddress(ipAddress, port), CONNECTION_TIMEOUT_MS)
                    candidate.soTimeout = COMMAND_TIMEOUT_MS
                    socket = candidate
                    connectionState = ConnectionState.Connected(ipAddress, port)
                    Result.success(Unit)
                } catch (error: Exception) {
                    try { candidate.close() } catch (_: IOException) {}
                    socket = null
                    connectionState = ConnectionState.Error("Błąd połączenia: ${error.message ?: error.javaClass.simpleName}")
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

    suspend fun sendCommand(command: String): Result<String> = withContext(Dispatchers.IO) {
        if (command.isBlank() || command.length > MAX_COMMAND_LENGTH ||
            command.contains('\n') || command.contains('\r')) {
            return@withContext Result.failure(
                IllegalArgumentException("Komenda jest pusta, zbyt długa lub zawiera znak końca linii")
            )
        }
        var notifyError = false
        val result = operationMutex.withLock {
            val active = socket
            if (active == null || active.isClosed || !active.isConnected ||
                connectionState !is ConnectionState.Connected) {
                return@withLock Result.failure(IllegalStateException("Brak aktywnego połączenia TCP"))
            }
            try {
                active.getOutputStream().apply {
                    write((command + "\n").toByteArray(StandardCharsets.UTF_8))
                    flush()
                }
                Result.success(readLegacyResponseFrame(active))
            } catch (error: Exception) {
                closeSocketLocked()
                connectionState = ConnectionState.Error("Błąd transmisji: ${error.message ?: error.javaClass.simpleName}")
                notifyError = true
                Result.failure(error)
            }
        }
        if (notifyError) notifyListeners(connectionState)
        result
    }

    private fun readLegacyResponseFrame(active: Socket): String {
        val input = active.getInputStream()
        val bytes = ByteArrayOutputStream()
        val buffer = ByteArray(512)
        var receivedAny = false
        while (true) {
            try {
                val count = input.read(buffer)
                if (count < 0) {
                    if (!receivedAny) throw IOException("Połączenie zamknięte bez odpowiedzi")
                    break
                }
                receivedAny = true
                if (bytes.size() + count > MAX_RESPONSE_BYTES) {
                    throw IOException("Odpowiedź przekracza limit ${MAX_RESPONSE_BYTES} bajtów")
                }
                bytes.write(buffer, 0, count)
            } catch (timeout: SocketTimeoutException) {
                if (!receivedAny) throw IOException("Przekroczono czas oczekiwania na odpowiedź", timeout)
                break
            }
        }
        val frame = bytes.toByteArray()
        if (frame.isEmpty()) throw IOException("Pusta odpowiedź")
        if (frame.last() != '\n'.code.toByte()) throw IOException("Niekompletna ramka: brak końcowego LF")
        val decoder = StandardCharsets.UTF_8.newDecoder()
            .onMalformedInput(CodingErrorAction.REPORT)
            .onUnmappableCharacter(CodingErrorAction.REPORT)
        val text = decoder.decode(ByteBuffer.wrap(frame)).toString()
        if (text.any { it == '\u0000' }) throw IOException("Odpowiedź zawiera bajt NUL")
        return text
    }

    private fun closeSocketLocked() {
        val old = socket
        socket = null
        try { old?.close() } catch (_: IOException) {}
    }

    suspend fun getStatus(): Result<SystemStatus> =
        sendCommand(CMD_STATUS).mapCatching { parseStatusResponse(it) }

    suspend fun configureChannel(config: ChannelConfig): Result<Unit> =
        Result.failure(UnsupportedOperationException(
            "Konfiguracja kanałów wyłączona: brak zweryfikowanego protokołu TCP w firmware"
        ))

    suspend fun startTherapy(): Result<Unit> =
        Result.failure(UnsupportedOperationException(
            "Start wyjść wyłączony do czasu wdrożenia protokołu i walidacji zabezpieczeń sprzętowych"
        ))

    suspend fun stopTherapy(): Result<Unit> =
        Result.failure(UnsupportedOperationException(
            "Sterowanie wyjściami wyłączone do czasu wdrożenia zweryfikowanego protokołu"
        ))

    suspend fun scanDevices(): Result<EffectorType> =
        sendCommand(CMD_DEVICE_SCAN).mapCatching { parseEffectorFromResponse(it) }

    suspend fun getLogs(): Result<List<LogEntry>> =
        sendCommand(CMD_LOGS).mapCatching { parseLogsResponse(it) }

    fun addConnectionListener(listener: ConnectionStateListener) { listeners.addIfAbsent(listener) }
    fun removeConnectionListener(listener: ConnectionStateListener) { listeners.remove(listener) }

    private fun notifyListeners(state: ConnectionState) {
        listeners.forEach { listener ->
            try { listener.onConnectionStateChanged(state) } catch (_: Exception) {
                // Błąd obserwatora nie może przerwać obsługi pozostałych.
            }
        }
    }

    private fun parseStatusResponse(response: String): SystemStatus {
        val fields = linkedMapOf<String, String>()
        val lines = response.lineSequence().toList()
        var index = 0
        while (index < lines.size) {
            val line = lines[index]
            val split = line.indexOf(':')
            if (split > 0) {
                val key = line.substring(0, split).trim()
                if (key in setOf("Uptime", "Free Memory", "Temperature", "PWM Running",
                        "Frequency", "Network", "Effector", "State", "System State", "Safety State")) {
                    var value = line.substring(split + 1).trim()
                    // The current sketch prints "Network: " and its value on the next line.
                    if (key == "Network" && value.isEmpty() && index + 1 < lines.size) {
                        value = lines[++index].trim()
                    }
                    if (fields.put(key, value) != null)
                        throw IOException("Powtórzone pole statusu: ${key}")
                }
            }
            index++
        }
        fun required(key: String): String = fields[key]?.takeIf { it.isNotBlank() }
            ?: throw IOException("Brak wymaganego pola statusu: ${key}")

        val uptime = required("Uptime").removeSuffix("s").trim().toLongOrNull()
            ?: throw IOException("Nieprawidłowe Uptime")
        val memory = required("Free Memory").removeSuffix("bytes").trim().toIntOrNull()
            ?: throw IOException("Nieprawidłowe Free Memory")
        val temperature = required("Temperature").removeSuffix("C").trim().toFloatOrNull()
            ?: throw IOException("Nieprawidłowa Temperature")
        val pwm = when (required("PWM Running").uppercase()) {
            "YES" -> true
            "NO" -> false
            else -> throw IOException("Nieprawidłowe PWM Running")
        }
        val network = when (required("Network").uppercase()) {
            "CONNECTED" -> true
            "DISCONNECTED" -> false
            else -> throw IOException("Nieprawidłowe Network")
        }
        if (uptime < 0 || memory < 0 || !temperature.isFinite())
            throw IOException("Wartości statusu poza zakresem")
        val frequency = fields["Frequency"]?.toFloatOrNull() ?: 0.0f
        if (!frequency.isFinite() || frequency < 0.0f) throw IOException("Nieprawidłowa Frequency")
        val state = (fields["System State"] ?: fields["State"])?.let(::parseSystemState)
            ?: SystemState.ERROR
        val effector = fields["Effector"]?.let(::parseEffector) ?: EffectorType.NONE
        return SystemStatus(
            uptimeSeconds = uptime, state = state, temperatureCelsius = temperature,
            freeMemoryBytes = memory, pwmIsActive = pwm, currentFrequency = frequency,
            networkConnected = network, detectedEffector = effector
        )
    }

    private fun parseSystemState(value: String): SystemState = when (value.trim().uppercase()) {
        "BOOT", "0" -> SystemState.BOOT
        "IDLE", "1" -> SystemState.IDLE
        "CONFIGURING", "CONFIGURATION", "2" -> SystemState.CONFIGURING
        "RUNNING", "3" -> SystemState.RUNNING
        "ERROR", "4" -> SystemState.ERROR
        "SAFE_LOCKOUT", "5" -> SystemState.SAFE_LOCKOUT
        "THERMAL_SHUTDOWN", "6" -> SystemState.THERMAL_SHUTDOWN
        else -> throw IOException("Nieznany stan systemu: ${value}")
    }

    private fun parseEffector(value: String): EffectorType = when {
        value.contains("Helmholtz", true) -> EffectorType.HELMHOLTZ
        value.contains("Otic", true) -> EffectorType.OTIC
        value.contains("Contact", true) -> EffectorType.CONTACT
        value.contains("Wrap", true) -> EffectorType.WRAP
        value.contains("IR LED", true) -> EffectorType.IR_LED_STRIP
        value.contains("Piezo", true) -> EffectorType.PIEZO_SPEAKER
        value.contains("Vibrator", true) -> EffectorType.VIBRATOR
        value.equals("None", true) -> EffectorType.NONE
        else -> throw IOException("Nieznany typ efektora: ${value}")
    }

    private fun parseEffectorFromResponse(response: String): EffectorType {
        val line = response.lineSequence().firstOrNull { it.startsWith("Detected effector:") }
            ?: throw IOException("Brak pola Detected effector")
        return parseEffector(line.substringAfter(':').trim())
    }

    private fun parseLogsResponse(response: String): List<LogEntry> {
        val entries = mutableListOf<LogEntry>()
        response.lineSequence().forEach { line ->
            val value = line.trim()
            if (value.isEmpty() || value.startsWith("===")) return@forEach
            val level = when {
                value.contains("[FATAL]") -> LogLevel.FATAL
                value.contains("[ERROR]") -> LogLevel.ERROR
                value.contains("[WARNING]") -> LogLevel.WARNING
                value.contains("[INFO]") -> LogLevel.INFO
                value.contains("[DEBUG]") -> LogLevel.DEBUG
                else -> throw IOException("Nieznany format wpisu logu")
            }
            entries.add(LogEntry(System.currentTimeMillis(), level, value))
        }
        if (entries.isEmpty()) throw IOException("Odpowiedź nie zawiera wpisów logu")
        return entries
    }
}
