/**
 * @file logging_system.h
 * @brief Zaawansowany system logowania z ring bufferem - Nagłówki
 * @version 4.0 (Modular Architecture with Pseudo-threading)
 * 
 * System logowania medycznego zgodny z IEC 60601-1
 */

#ifndef LOGGING_SYSTEM_H
#define LOGGING_SYSTEM_H

#include <Arduino.h>
#include "types.h"
#include <avr/pgmspace.h>

// ============================================================================
// KONFIGURACJA LOGOWANIA
// ============================================================================

// Domyślny poziom logowania
#ifndef CURRENT_LOG_LEVEL
#define CURRENT_LOG_LEVEL LOG_VERBOSE
#endif

// Rozmiar bufora
#define LOG_BUFFER_SIZE 4
#define LOG_MESSAGE_MAX_LEN 24

// Flagi konfiguracyjne
#define ENABLE_SERIAL_LOG true
#define ENABLE_REMOTE_LOG true

// Struktura wpisu logu (używamy LogEntry_t z types.h)
typedef struct {
    uint32_t timestamp;
    uint8_t level;
    uint16_t event_code;
    char message[LOG_MESSAGE_MAX_LEN];
} LogEntry;

// Ring buffer logów
typedef struct {
    LogEntry entries[LOG_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    uint16_t overflow_count;
} LogBuffer;

// Funkcje publiczne
void logging_init();
void logging_loop();
void log_add_entry(uint8_t level, uint16_t event_code, const char* message);
void log_add_entry_P(uint8_t level, uint16_t event_code, PGM_P message);
const __FlashStringHelper* get_log_level_name(uint8_t level);
LogBuffer* logging_get_buffer();
void logging_flush_to_serial();
uint16_t logging_get_overflow_count();

// Makra pomocnicze
#define LOG_VERBOSE(msg) log_add_entry_P(LOG_VERBOSE, 0, PSTR(msg))
#define LOG_DEBUG(msg)   log_add_entry_P(LOG_DEBUG, 0, PSTR(msg))
#define LOG_INFO(msg)    log_add_entry_P(LOG_INFO, 0, PSTR(msg))
#define LOG_WARN(msg)    log_add_entry_P(LOG_WARNING, 0, PSTR(msg))
#define LOG_ERROR(msg)   log_add_entry_P(LOG_ERROR, 0, PSTR(msg))
#define LOG_FATAL(msg)   log_add_entry_P(LOG_FATAL, 0, PSTR(msg))

#define LOG_EVENT(level, event, msg) log_add_entry(level, event, msg)

#define LOG_WARNING(msg) LOG_WARN(msg)

#endif // LOGGING_SYSTEM_H
