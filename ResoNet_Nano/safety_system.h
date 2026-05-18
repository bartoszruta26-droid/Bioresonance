/**
 * @file safety_system.h
 * @brief System bezpieczeństwa medycznego - Nagłówki
 * @version 4.0 (Modular Architecture with Pseudo-threading)
 * 
 * Bezpieczeństwo zgodne z IEC 60601-1
 */

#ifndef SAFETY_SYSTEM_H
#define SAFETY_SYSTEM_H

#include <Arduino.h>
#include <avr/wdt.h>

// ============================================================================
// KONFIGURACJA WATCHDOGA
// ============================================================================

#define WDT_TIMEOUT_MS 2000
#define WDT_RESET_THRESHOLD 5
#define SAFE_MODE_THRESHOLD 3

// Poziomy monitorowania Watchdog
#define WDT_LAYER_MAIN      0x01
#define WDT_LAYER_NETWORK   0x02
#define WDT_LAYER_THERAPY   0x04
#define WDT_LAYER_COMMS     0x08

// Flagi stanu bezpieczeństwa
#define SAFE_STATE_OK           0x00
#define SAFE_STATE_WARNING      0x01
#define SAFE_STATE_ERROR        0x02
#define SAFE_STATE_CRITICAL     0x03
#define SAFE_STATE_LOCKOUT      0x04

// Kody błędów krytycznych
#define SAFE_ERROR_NONE             0x00
#define SAFE_ERROR_WDTO_RESET       0x01
#define SAFE_ERROR_FREQ_FAULT       0x02
#define SAFE_ERROR_OVERHEAT         0x03
#define SAFE_ERROR_CURRENT_FAULT    0x04
#define SAFE_ERROR_WATCHDOG_FAIL    0x05
#define SAFE_ERROR_EEPROM_FAIL      0x06
#define SAFE_ERROR_STACK_OVERFLOW   0x07
#define SAFE_ERROR_HEAP_CORRUPT     0x08

// Struktura stanu systemu bezpieczeństwa
typedef struct {
    uint8_t safe_state;
    uint8_t error_code;
    uint16_t reset_count;
    uint32_t last_feed_time;
    uint8_t layer_status;
    uint16_t temperature_x10;
    uint32_t uptime_ms;
} SafetyStatus;

// Funkcje publiczne
void safety_init();
void safety_loop();
void safety_feed(uint8_t layer);
bool safety_is_safe();
uint8_t safety_get_error();
void safety_trigger_emergency();
SafetyStatus* safety_get_status();
void safety_clear_latch();
uint16_t safety_get_temperature();
bool safety_check_thermal();

// Makra bezpieczeństwa
#define SAFE_FEED(layer) safety_feed(layer)
#define SAFE_IS_SAFE() safety_is_safe()
#define SAFE_EMERGENCY() safety_trigger_emergency()
#define SAFE_GET_ERROR() safety_get_error()

#endif // SAFETY_SYSTEM_H
