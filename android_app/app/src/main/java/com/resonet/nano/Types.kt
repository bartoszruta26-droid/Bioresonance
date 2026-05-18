package com.resonet.nano

/**
 * Typy efektorów obsługiwanych przez system ResoNet Nano
 * Zgodne z types.h i device_detector.h
 */
enum class EffectorType(val id: Int, val displayName: String) {
    NONE(0, "Brak efektora"),
    HELMHOLTZ(1, "Cewka Helmholtza"),
    OTIC(2, "Aplikator Uszny"),
    CONTACT(3, "Elektrody Kontaktowe"),
    WRAP(4, "Aplikator Okrężny"),
    IR_LED_STRIP(5, "Pasek LED IR"),
    PIEZO_SPEAKER(6, "Głośnik Piezo/Audio"),
    VIBRATOR(7, "Wibrator");

    companion object {
        fun fromId(id: Int): EffectorType {
            return values().find { it.id == id } ?: NONE
        }
    }
}

/**
 * Typy modulacji sygnału
 * Zgodne z types.h - ModulationType_t
 */
enum class ModulationType(val id: Int, val displayName: String) {
    NONE(0, "Brak"),
    AM(1, "AM"),
    FM(2, "FM"),
    BURST(3, "Burst");

    companion object {
        fun fromId(id: Int): ModulationType {
            return values().find { it.id == id } ?: NONE
        }
    }
}

/**
 * Stany systemu
 * Zgodne z types.h - SystemState_t
 */
enum class SystemState(val id: Int, val displayName: String) {
    BOOT(0, "Uruchamianie"),
    IDLE(1, "Gotowy"),
    CONFIGURING(2, "Konfiguracja"),
    RUNNING(3, "Praca"),
    ERROR(4, "Błąd"),
    SAFE_LOCKOUT(5, "Blokada Bezpieczeństwa"),
    THERMAL_SHUTDOWN(6, "Wyłączenie Termiczne");

    companion object {
        fun fromId(id: Int): SystemState {
            return values().find { it.id == id } ?: ERROR
        }
    }
}

/**
 * Poziomy logowania
 * Zgodne z types.h - LogLevel_t
 */
enum class LogLevel(val id: Int, val displayName: String) {
    VERBOSE(0, "Verbose"),
    DEBUG(1, "Debug"),
    INFO(2, "Info"),
    WARNING(3, "Ostrzeżenie"),
    ERROR(4, "Błąd"),
    FATAL(5, "Krytyczny");

    companion object {
        fun fromId(id: Int): LogLevel {
            return values().find { it.id == id } ?: INFO
        }
    }
}

/**
 * Typy zdarzeń systemowych
 * Zgodne z types.h - EventType_t
 */
enum class EventType(val id: Int, val displayName: String) {
    NONE(0, "Brak"),
    CMD_RECEIVED(1, "Komenda Odebrana"),
    CMD_EXECUTED(2, "Komenda Wykonana"),
    CMD_FAILED(3, "Komenda Nieudana"),
    NETWORK_CONNECT(4, "Łączenie z Siecią"),
    NETWORK_DISCONNECT(5, "Rozłączanie z Siecią"),
    PWM_START(6, "Start PWM"),
    PWM_STOP(7, "Stop PWM"),
    SAFETY_TRIP(8, "Wyzwolenie Bezpieczeństwa"),
    THERMAL_WARN(9, "Ostrzeżenie Termiczne"),
    SYSTEM_RESET(10, "Reset Systemu"),
    THERAPY_START(11, "Start Terapii"),
    THERAPY_STOP(12, "Stop Terapii"),
    THERAPY_COMPLETE(13, "Terapia Zakończona");

    companion object {
        fun fromId(id: Int): EventType {
            return values().find { it.id == id } ?: NONE
        }
    }
}
