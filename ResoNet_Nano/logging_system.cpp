/**
 * @file logging_system.cpp
 * @brief Zaawansowany system logowania z ring bufferem - Implementacja
 * @version 4.0 (Modular Architecture with Pseudo-threading)
 */

#include "logging_system.h"

// ============================================================================
// ZMIENNE PRYWATNE
// ============================================================================

static LogBuffer log_buffer;
static const char* log_level_names[] = {
    "VERBOSE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

// ============================================================================
// IMPLEMENTACJA FUNKCJI PUBLICZNYCH
// ============================================================================

/**
 * @brief Inicjalizuje system logowania
 */
void logging_init() {
    cli();
    log_buffer.head = 0;
    log_buffer.tail = 0;
    log_buffer.count = 0;
    log_buffer.overflow_count = 0;
    sei();
    
    LOG_INFO("Logging system initialized");
}

/**
 * @brief Pętla systemu logowania (pseudo-wątek)
 * Odpowiedzialna za flushowanie bufora
 */
void logging_loop() {
    // W tej wersji logi są zapisywane natychmiast na Serial
    // W przyszłości można dodać asynchroniczne flushowanie
    
    static uint32_t last_stats_log = 0;
    uint32_t now = millis();
    
    // Loguj statystyki co 60 sekund
    if (now - last_stats_log >= 60000) {
        last_stats_log = now;
        
        if (log_buffer.overflow_count > 0) {
            LOG_WARN_F("Log buffer overflow: %u entries lost", log_buffer.overflow_count);
        }
    }
}

/**
 * @brief Dodaje wpis do ring buffera logów
 * @param level Poziom logowania
 * @param event_code Kod zdarzenia
 * @param message Wiadomość tekstowa
 */
void log_add_entry(uint8_t level, uint16_t event_code, const char* message) {
    if (level < CURRENT_LOG_LEVEL) {
        return;  // Pomijamy logi poniżej aktualnego poziomu
    }
    
    cli();
    
    // Sprawdź przepełnienie
    if (log_buffer.count >= LOG_BUFFER_SIZE) {
        log_buffer.overflow_count++;
        log_buffer.tail = (log_buffer.tail + 1) % LOG_BUFFER_SIZE;
        log_buffer.count--;
    }
    
    // Dodaj nowy wpis
    LogEntry* entry = &log_buffer.entries[log_buffer.head];
    entry->timestamp = millis();
    entry->level = level;
    entry->event_code = event_code;
    
    // Skopiuj wiadomość (bezpieczne kopiowanie)
    strncpy(entry->message, message, sizeof(entry->message) - 1);
    entry->message[sizeof(entry->message) - 1] = '\0';
    
    // Aktualizuj wskaźniki
    log_buffer.head = (log_buffer.head + 1) % LOG_BUFFER_SIZE;
    log_buffer.count++;
    
    sei();
    
    // Wyślij na Serial jeśli włączone i Serial jest dostępny
    if (ENABLE_SERIAL_LOG && Serial) {
        Serial.print(F("[LOG "));
        Serial.print(millis());
        Serial.print(F("ms] "));
        Serial.print(get_log_level_name(level));
        Serial.print(F(": "));
        Serial.println(message);
    }
}

/**
 * @brief Pobiera nazwę poziomu logowania
 * @param level Poziom logowania
 * @return Nazwa jako string z pamięci FLASH
 */
const char* get_log_level_name(uint8_t level) {
    if (level <= LOG_FATAL) {
        return log_level_names[level];
    }
    return "UNKNOWN";
}

/**
 * @brief Pobiera wskaźnik do bufora logów
 * @return Wskaźnik do struktury LogBuffer
 */
LogBuffer* logging_get_buffer() {
    return &log_buffer;
}

/**
 * @brief Wymusza zapisanie wszystkich logów na Serial
 */
void logging_flush_to_serial() {
    cli();
    uint16_t count = log_buffer.count;
    uint16_t tail = log_buffer.tail;
    sei();
    
    while (count > 0) {
        LogEntry* entry = &log_buffer.entries[tail];
        
        Serial.print(F("[HIST "));
        Serial.print(entry->timestamp);
        Serial.print(F("ms] "));
        Serial.print(get_log_level_name(entry->level));
        Serial.print(F(": "));
        Serial.println(entry->message);
        
        tail = (tail + 1) % LOG_BUFFER_SIZE;
        count--;
        
        // Krótka przerwa żeby nie zablokować innych zadań
        delay(1);
    }
}

/**
 * @brief Pobiera liczbę przepełnień bufora
 * @return Liczba utraconych wpisów
 */
uint16_t logging_get_overflow_count() {
    return log_buffer.overflow_count;
}
