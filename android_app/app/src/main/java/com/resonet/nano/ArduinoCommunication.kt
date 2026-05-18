package com.resonet.nano

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.BufferedReader
import java.io.InputStreamReader
import java.io.PrintWriter
import java.net.Socket

/**
 * Interfejs komunikacyjny dla Arduino Nano z Ethernet HAT
 * Implementuje protokół TCP zgodny z webui i bashtui
 */
class ArduinoCommunication {
    
    companion object {
        const val DEFAULT_PORT = 5001
        const val CONNECTION_TIMEOUT_MS = 3000
        const val COMMAND_TIMEOUT_MS = 2000
        
        // Komendy systemu zgodne z .ino
        const val CMD_STATUS = "s"
        const val CMD_START_THERAPY = "START"
        const val CMD_STOP_THERAPY = "STOP"
        const val CMD_LOGS = "l"
        const val CMD_EVENT_STATS = "e"
        const val CMD_DEVICE_SCAN = "d"
    }
    
    private var socket: Socket? = null
    private var writer: PrintWriter? = null
    private var reader: BufferedReader? = null
    
    var connectionState: ConnectionState = ConnectionState.Disconnected
        private set
    
    private val listeners = mutableListOf<ConnectionStateListener>()
    
    interface ConnectionStateListener {
        fun onConnectionStateChanged(state: ConnectionState)
    }
    
    /**
     * Łączy się z urządzeniem Arduino przez TCP
     */
    suspend fun connect(ipAddress: String, port: Int = DEFAULT_PORT): Result<Unit> = withContext(Dispatchers.IO) {
        try {
            connectionState = ConnectionState.Connecting
            notifyListeners(ConnectionState.Connecting)
            
            socket = Socket().apply {
                soTimeout = CONNECTION_TIMEOUT_MS
                connect(java.net.InetSocketAddress(ipAddress, port), CONNECTION_TIMEOUT_MS)
            }
            
            writer = PrintWriter(socket!!.getOutputStream(), true)
            reader = BufferedReader(InputStreamReader(socket!!.getInputStream()))
            
            connectionState = ConnectionState.Connected(ipAddress, port)
            notifyListeners(connectionState)
            
            Result.success(Unit)
        } catch (e: Exception) {
            connectionState = ConnectionState.Error("Błąd połączenia: ${e.message}")
            notifyListeners(connectionState)
            Result.failure(e)
        }
    }
    
    /**
     * Rozłącza z urządzeniem
     */
    suspend fun disconnect(): Unit = withContext(Dispatchers.IO) {
        try {
            reader?.close()
            writer?.close()
            socket?.close()
        } catch (e: Exception) {
            // Ignoruj błędy rozłączania
        } finally {
            reader = null
            writer = null
            socket = null
            connectionState = ConnectionState.Disconnected
            notifyListeners(ConnectionState.Disconnected)
        }
    }
    
    /**
     * Wysyła komendę i odbiera odpowiedź
     */
    suspend fun sendCommand(command: String): Result<String> = withContext(Dispatchers.IO) {
        if (connectionState !is ConnectionState.Connected) {
            return@withContext Result.failure(IllegalStateException("Brak połączenia"))
        }
        
        try {
            writer?.println(command)
            writer?.flush()
            
            // Odczytaj odpowiedź z timeoutem
            val response = StringBuilder()
            val startTime = System.currentTimeMillis()
            
            while (System.currentTimeMillis() - startTime < COMMAND_TIMEOUT_MS) {
                if (reader?.ready() == true) {
                    val line = reader?.readLine()
                    if (line != null) {
                        response.appendLine(line)
                        // Sprawdź czy to koniec odpowiedzi (OK/DONE lub limit długości)
                        // Nie przerywaj na liniach nagłówków ===
                        if ((line.contains("OK") || line.contains("DONE")) && 
                            response.length > 4096) {
                            break
                        }
                    }
                } else {
                    Thread.sleep(10)
                }
            }
            
            Result.success(response.toString())
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
    
    /**
     * Pobiera status systemu
     */
    suspend fun getStatus(): Result<SystemStatus> = withContext(Dispatchers.IO) {
        sendCommand(CMD_STATUS).mapCatching { response ->
            parseStatusResponse(response)
        }
    }
    
    /**
     * Konfiguruje kanał
     */
    suspend fun configureChannel(config: ChannelConfig): Result<Unit> = withContext(Dispatchers.IO) {
        val validation = config.validate()
        if (!validation.isValid) {
            return@withContext Result.failure(IllegalArgumentException(validation.errorMessage))
        }
        
        sendCommand(config.toCommandString()).map {
            // Sukces konfiguracji
        }
    }
    
    /**
     * Start terapii
     */
    suspend fun startTherapy(): Result<Unit> = withContext(Dispatchers.IO) {
        sendCommand(CMD_START_THERAPY).map {
            // Sukces startu
        }
    }
    
    /**
     * Stop terapii
     */
    suspend fun stopTherapy(): Result<Unit> = withContext(Dispatchers.IO) {
        sendCommand(CMD_STOP_THERAPY).map {
            // Sukces stopu
        }
    }
    
    /**
     * Skanowanie urządzeń (efektory/sensory)
     */
    suspend fun scanDevices(): Result<EffectorType> = withContext(Dispatchers.IO) {
        sendCommand(CMD_DEVICE_SCAN).mapCatching { response ->
            parseEffectorFromResponse(response)
        }
    }
    
    /**
     * Pobiera logi systemowe
     */
    suspend fun getLogs(): Result<List<LogEntry>> = withContext(Dispatchers.IO) {
        sendCommand(CMD_LOGS).mapCatching { response ->
            parseLogsResponse(response)
        }
    }
    
    /**
     * Dodaje nasłuchiwacza zmian stanu połączenia
     */
    fun addConnectionListener(listener: ConnectionStateListener) {
        listeners.add(listener)
    }
    
    /**
     * Usuwa nasłuchiwacza
     */
    fun removeConnectionListener(listener: ConnectionStateListener) {
        listeners.remove(listener)
    }
    
    private fun notifyListeners(state: ConnectionState) {
        listeners.forEach { it.onConnectionStateChanged(state) }
    }
    
    /**
     * Parsuje odpowiedź statusu z Arduino
     * Format: === System Status ===\nUptime: 100s\nFree Memory: 1234 bytes\n...
     */
    private fun parseStatusResponse(response: String): SystemStatus {
        val lines = response.split("\n")
        var uptime = 0L
        var temp = 0.0f
        var freeMem = 0
        var pwmActive = false
        var freq = 0.0f
        var networkConn = false
        var effector = EffectorType.NONE
        var state = SystemState.IDLE
        
        for (line in lines) {
            when {
                line.contains("Uptime:") -> {
                    uptime = line.substringAfter("Uptime:").substringBefore("s").trim().toLongOrNull() ?: 0L
                }
                line.contains("Temperature:") -> {
                    temp = line.substringAfter("Temperature:").substringBefore("C").trim().toFloatOrNull() ?: 0.0f
                }
                line.contains("Free Memory:") -> {
                    freeMem = line.substringAfter("Free Memory:").substringBefore("bytes").trim().toIntOrNull() ?: 0
                }
                line.contains("PWM Running:") -> {
                    pwmActive = line.contains("YES")
                }
                line.contains("Frequency:") -> {
                    freq = line.substringAfter("Frequency:").trim().toFloatOrNull() ?: 0.0f
                }
                line.contains("Network:") -> {
                    networkConn = line.contains("CONNECTED")
                }
                line.contains("Effector:") -> {
                    effector = parseEffectorFromString(line)
                }
                line.contains("State:") || line.contains("System State:") -> {
                    state = parseSystemStateFromString(line)
                }
                line.contains("Safety State:") -> {
                    state = parseSystemStateFromString(line)
                }
            }
        }
        
        return SystemStatus(
            uptimeSeconds = uptime,
            state = state,
            temperatureCelsius = temp,
            freeMemoryBytes = freeMem,
            pwmIsActive = pwmActive,
            currentFrequency = freq,
            networkConnected = networkConn,
            detectedEffector = effector
        )
    }
    
    /**
     * Parsuje stan systemu z odpowiedzi
     */
    private fun parseSystemStateFromString(line: String): SystemState {
        return when {
            line.contains("BOOT") -> SystemState.BOOT
            line.contains("IDLE") -> SystemState.IDLE
            line.contains("CONFIGUR") -> SystemState.CONFIGURING
            line.contains("RUNNING") || line.contains("Praca") -> SystemState.RUNNING
            line.contains("ERROR") || line.contains("Błąd") -> SystemState.ERROR
            line.contains("SAFE_LOCKOUT") || line.contains("Blokada Bezpieczeństwa") -> SystemState.SAFE_LOCKOUT
            line.contains("THERMAL") || line.contains("Wyłączenie Termiczne") -> SystemState.THERMAL_SHUTDOWN
            else -> SystemState.IDLE
        }
    }
    
    /**
     * Parsuje typ efektora z odpowiedzi
     */
    private fun parseEffectorFromString(line: String): EffectorType {
        return when {
            line.contains("Helmholtz") -> EffectorType.HELMHOLTZ
            line.contains("Otic") -> EffectorType.OTIC
            line.contains("Contact") -> EffectorType.CONTACT
            line.contains("Wrap") -> EffectorType.WRAP
            line.contains("IR LED") -> EffectorType.IR_LED_STRIP
            line.contains("Piezo") -> EffectorType.PIEZO_SPEAKER
            line.contains("Vibrator") -> EffectorType.VIBRATOR
            else -> EffectorType.NONE
        }
    }
    
    /**
     * Parsuje wynik skanowania urządzeń
     */
    private fun parseEffectorFromResponse(response: String): EffectorType {
        val lines = response.split("\n")
        for (line in lines) {
            if (line.contains("Detected effector:")) {
                return parseEffectorFromString(line)
            }
        }
        return EffectorType.NONE
    }
    
    /**
     * Parsuje logi systemowe
     */
    private fun parseLogsResponse(response: String): List<LogEntry> {
        val entries = mutableListOf<LogEntry>()
        val lines = response.split("\n")
        
        for (line in lines) {
            if (line.trim().isEmpty() || line.startsWith("===")) continue
            
            // Proste parsowanie - w produkcji użyj regex
            val level = when {
                line.contains("[ERROR]") -> LogLevel.ERROR
                line.contains("[WARNING]") -> LogLevel.WARNING
                line.contains("[INFO]") -> LogLevel.INFO
                line.contains("[DEBUG]") -> LogLevel.DEBUG
                else -> LogLevel.INFO
            }
            
            entries.add(LogEntry(
                timestamp = System.currentTimeMillis(),
                level = level,
                message = line
            ))
        }
        
        return entries
    }
}
