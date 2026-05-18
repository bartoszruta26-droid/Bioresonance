package com.resonet.nano

/**
 * Model danych dla pojedynczego kanału/efektora
 * Odpowiednik struktury konfiguracji z webui i bashtui
 */
data class ChannelConfig(
    val id: Int,                    // Numer kanału 1-8
    var name: String = "",          // Nazwa opisowa
    var effectorType: EffectorType = EffectorType.NONE,
    var frequency: Float = 727.0f,  // Częstotliwość w Hz
    var dutyCycle: Int = 50,        // Cykl pracy 0-100%
    var intensity: Int = 2048,      // Intensywność 0-4095
    var modulation: ModulationType = ModulationType.NONE,
    var enabled: Boolean = false,   // Czy kanał aktywny
    var isRunning: Boolean = false  // Czy aktualnie generuje sygnał
) {
    /**
     * Konwertuje do formatu komendy TCP dla Arduino
     * Format: CONFIG:channel,freq_x100,duty,intensity,modulation
     */
    fun toCommandString(): String {
        val freqInt = (frequency * 100).toInt()
        return "CONFIG:$id,$freqInt,$dutyCycle,$intensity,${modulation.id}"
    }

    /**
     * Waliduje poprawność konfiguracji
     */
    fun validate(): ValidationResult {
        if (frequency < 0.1f || frequency > 500000.0f) {
            return ValidationResult(false, "Częstotliwość poza zakresem 0.1-500000 Hz")
        }
        if (dutyCycle < 0 || dutyCycle > 100) {
            return ValidationResult(false, "Cykl pracy poza zakresem 0-100%")
        }
        if (intensity < 0 || intensity > 4095) {
            return ValidationResult(false, "Intensywność poza zakresem 0-4095")
        }
        return ValidationResult(true, null)
    }
}

/**
 * Wynik walidacji konfiguracji
 */
data class ValidationResult(
    val isValid: Boolean,
    val errorMessage: String?
)

/**
 * Status systemu Arduino
 * Odpowiednik struktury SystemStatus_t z types.h
 */
data class SystemStatus(
    val uptimeSeconds: Long = 0L,
    val state: SystemState = SystemState.IDLE,
    val temperatureCelsius: Float = 0.0f,
    val freeMemoryBytes: Int = 0,
    val watchdogResets: Int = 0,
    val eventsProcessed: Long = 0L,
    val errorsTotal: Int = 0,
    val pwmIsActive: Boolean = false,
    val currentFrequency: Float = 0.0f,
    val networkConnected: Boolean = false,
    val detectedEffector: EffectorType = EffectorType.NONE,
    val biofeedbackEnabled: Boolean = false
) {
    val isSafe: Boolean
        get() = state == SystemState.IDLE || state == SystemState.RUNNING
    
    val hasError: Boolean
        get() = state == SystemState.ERROR || state == SystemState.SAFE_LOCKOUT || 
                state == SystemState.THERMAL_SHUTDOWN
}

/**
 * Wpis logu systemowego
 * Odpowiednik LogEntry_t z types.h
 */
data class LogEntry(
    val timestamp: Long,
    val level: LogLevel,
    val message: String
)

/**
 * Zdarzenie systemowe
 * Odpowiednik Event_t z types.h
 */
data class SystemEvent(
    val timestamp: Long,
    val type: EventType,
    val data: Int = 0,
    val severity: Int = 0
)

/**
 * Stan połączenia z urządzeniem
 */
sealed class ConnectionState {
    object Disconnected : ConnectionState()
    object Connecting : ConnectionState()
    data class Connected(val ipAddress: String, val port: Int) : ConnectionState()
    data class Error(val message: String) : ConnectionState()
}

/**
 * Tryb pracy systemu
 * Zgodne z bash_tui i webui
 */
enum class ProbeMode(val displayName: String) {
    SINGLE("Pojedyncza"),
    DUAL_INDEPENDENT("Dual Niezależny"),
    DUAL_SYNC("Dual Sync"),
    MULTI_CHANNEL("Wielokanałowa"),
    SEQUENTIAL("Sekwencyjna")
}
