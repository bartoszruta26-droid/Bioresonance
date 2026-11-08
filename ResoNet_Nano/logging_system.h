/**
 * @file logging_system.h
 * @brief Compact compile-time configurable logging interface.
 */
#ifndef LOGGING_SYSTEM_H
#define LOGGING_SYSTEM_H

#include <Arduino.h>
#include "types.h"
#include <avr/pgmspace.h>

// Numeric levels match LOG_VERBOSE..LOG_FATAL in types.h.
// Default firmware profile keeps warnings and errors; set 0 for full diagnostics.
#ifndef RESONET_LOG_LEVEL
#define RESONET_LOG_LEVEL 3
#endif
#ifndef CURRENT_LOG_LEVEL
#define CURRENT_LOG_LEVEL RESONET_LOG_LEVEL
#endif

#define LOG_BUFFER_SIZE 4
#define LOG_MESSAGE_MAX_LEN 24
#define ENABLE_SERIAL_LOG true
#define ENABLE_REMOTE_LOG true

typedef struct {
    uint32_t timestamp;
    uint8_t level;
    uint16_t event_code;
    char message[LOG_MESSAGE_MAX_LEN];
} LogEntry;

typedef struct {
    LogEntry entries[LOG_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    uint16_t overflow_count;
} LogBuffer;

void logging_init();
void logging_loop();
void log_add_entry(uint8_t level, uint16_t event_code, const char* message);
void log_add_entry_P(uint8_t level, uint16_t event_code, PGM_P message);
const __FlashStringHelper* get_log_level_name(uint8_t level);
LogBuffer* logging_get_buffer();
void logging_flush_to_serial();
uint16_t logging_get_overflow_count();

// Compile out disabled log call sites and their flash-resident strings.
#if RESONET_LOG_LEVEL <= 0
#define LOG_VERBOSE(msg) log_add_entry_P(LOG_VERBOSE, 0, PSTR(msg))
#else
#define LOG_VERBOSE(msg) ((void)0)
#endif
#if RESONET_LOG_LEVEL <= 1
#define LOG_DEBUG(msg) log_add_entry_P(LOG_DEBUG, 0, PSTR(msg))
#else
#define LOG_DEBUG(msg) ((void)0)
#endif
#if RESONET_LOG_LEVEL <= 2
#define LOG_INFO(msg) log_add_entry_P(LOG_INFO, 0, PSTR(msg))
#else
#define LOG_INFO(msg) ((void)0)
#endif
#if RESONET_LOG_LEVEL <= 3
#define LOG_WARN(msg) log_add_entry_P(LOG_WARNING, 0, PSTR(msg))
#else
#define LOG_WARN(msg) ((void)0)
#endif
#if RESONET_LOG_LEVEL <= 4
#define LOG_ERROR(msg) log_add_entry_P(LOG_ERROR, 0, PSTR(msg))
#else
#define LOG_ERROR(msg) ((void)0)
#endif
#if RESONET_LOG_LEVEL <= 5
#define LOG_FATAL(msg) log_add_entry_P(LOG_FATAL, 0, PSTR(msg))
#else
#define LOG_FATAL(msg) ((void)0)
#endif

#define LOG_WARNING(msg) LOG_WARN(msg)
#define LOG_EVENT(level, event, msg) log_add_entry(level, event, msg)

#endif // LOGGING_SYSTEM_H
