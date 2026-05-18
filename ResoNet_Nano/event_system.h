/**
 * @file event_system.h
 * @brief System obsługi zdarzeń (Event Handling) - Nagłówki
 * @version 4.1 (Poprawiona spójność typów)
 */

#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <Arduino.h>
#include "types.h"

// ============================================================================
// KONFIGURACJA SYSTEMU ZDARZEŃ
// ============================================================================

#define EVENT_QUEUE_SIZE 32
#define EVENT_DESC_MAX_LEN 32

// Poziomy ważności zdarzeń (używamy z types.h)
#define EVENT_SEVERITY_INFO      0
#define EVENT_SEVERITY_WARNING   1
#define EVENT_SEVERITY_ERROR     2
#define EVENT_SEVERITY_CRITICAL  3

// Struktura zdarzenia systemowego (używamy Event_t z types.h jako bazę)
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
#define EVENT_INFO(code, desc)      event_push(code, 0, 0, desc)
#define EVENT_WARNING(code, desc)   event_push(code, 1, 0, desc)
#define EVENT_ERROR(code, desc)     event_push(code, 2, 0, desc)
#define EVENT_CRITICAL(code, desc)  event_push(code, 3, 0, desc)
#define EVENT_DATA(code, data, desc) event_push(code, 0, data, desc)

#endif // EVENT_SYSTEM_H
