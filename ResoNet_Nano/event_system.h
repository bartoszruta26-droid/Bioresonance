/**
 * @file event_system.h
 * @brief System obsługi zdarzeń (Event Handling) - Nagłówki
 * @version 4.0 (Modular Architecture with Pseudo-threading)
 */

#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <Arduino.h>

// ============================================================================
// KONFIGURACJA SYSTEMU ZDARZEŃ
// ============================================================================

#define EVENT_QUEUE_SIZE 32
#define EVENT_DESC_MAX_LEN 32

// Kody zdarzeń systemowych
#define EVENT_NONE                  0x0000
#define EVENT_THERAPY_START         0x0001
#define EVENT_THERAPY_STOP          0x0002
#define EVENT_THERAPY_COMPLETE      0x0003
#define EVENT_FREQ_CHANGED          0x0004
#define EVENT_MODULATION_CHANGED    0x0005
#define EVENT_NETWORK_CONNECTED     0x0006
#define EVENT_NETWORK_DISCONNECTED  0x0007
#define EVENT_WATCHDOG_FEED         0x0008
#define EVENT_ERROR_OCCURRED        0x0009
#define EVENT_LOG_FLUSH             0x000A
#define EVENT_SYSTEM_RESET          0x000B
#define EVENT_SAFETY_TRIGGER        0x000C
#define EVENT_PACKET_RECEIVED       0x000D
#define EVENT_PACKET_SENT           0x000E
#define EVENT_CALIBRATION_START     0x000F
#define EVENT_CALIBRATION_END       0x0010

// Poziomy ważności zdarzeń
#define EVENT_SEVERITY_INFO         0
#define EVENT_SEVERITY_WARNING      1
#define EVENT_SEVERITY_ERROR        2
#define EVENT_SEVERITY_CRITICAL     3

// Struktura zdarzenia systemowego
typedef struct {
    uint32_t timestamp;
    uint16_t event_code;
    uint8_t severity;
    uint32_t data;
    char description[EVENT_DESC_MAX_LEN];
} SystemEvent;

// Kolejka zdarzeń FIFO
typedef struct {
    SystemEvent events[EVENT_QUEUE_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    uint16_t dropped_count;
} EventQueue;

// Funkcje publiczne
void event_system_init();
void event_system_loop();
bool event_push(uint16_t event_code, uint8_t severity, uint32_t data, const char* description);
bool event_pop(SystemEvent* event);
bool event_peek(SystemEvent* event);
uint16_t event_get_count();
uint16_t event_get_dropped_count();
void event_clear_queue();

// Makra pomocnicze
#define EVENT_INFO(code, desc)      event_push(code, EVENT_SEVERITY_INFO, 0, desc)
#define EVENT_WARNING(code, desc)   event_push(code, EVENT_SEVERITY_WARNING, 0, desc)
#define EVENT_ERROR(code, desc)     event_push(code, EVENT_SEVERITY_ERROR, 0, desc)
#define EVENT_CRITICAL(code, desc)  event_push(code, EVENT_SEVERITY_CRITICAL, 0, desc)
#define EVENT_DATA(code, data, desc) event_push(code, EVENT_SEVERITY_INFO, data, desc)

#endif // EVENT_SYSTEM_H
