/**
 * @file safety_system.cpp
 * @brief System bezpieczeństwa medycznego - Implementacja
 * @version 4.0 (Modular Architecture with Pseudo-threading)
 * 
 * Bezpieczeństwo zgodne z IEC 60601-1
 * - Watchdog Timer z wieloma warstwami
 * - Monitorowanie temperatury
 * - Emergency shutdown
 * - Historia resetów w EEPROM
 */

#include "safety_system.h"
#include <avr/eeprom.h>

// ============================================================================
// ZMIENNE PRYWATNE
// ============================================================================

static SafetyStatus safety_status;
static volatile uint32_t layer_timers[4];
static uint8_t wdt_reset_count = 0;
static bool emergency_latch = false;

// Adresy EEPROM
#define EEPROM_WDT_COUNT_ADDR 0
#define EEPROM_RESET_FLAG_ADDR 1

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

/**
 * @brief Odczytuje przyczynę resetu z MCUSR
 */
static uint8_t read_mcusr() {
    uint8_t mcusr = MCUSR;
    MCUSR = 0;  // Clear flag
    return mcusr;
}

/**
 * @brief Odczytuje liczbę resetów z EEPROM
 */
static uint8_t eeprom_read_reset_count() {
    return eeprom_read_byte((uint8_t*)EEPROM_WDT_COUNT_ADDR);
}

/**
 * @brief Zapisuje liczbę resetów do EEPROM
 */
static void eeprom_write_reset_count(uint8_t count) {
    eeprom_write_byte((uint8_t*)EEPROM_WDT_COUNT_ADDR, count);
}

/**
 * @brief Szacuje temperaturę MCU (przybliżona)
 */
static uint16_t measure_temperature() {
    // Użycie wewnętrznego referencyjnego źródła napięcia 1.1V
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    delay(5);  // Czekaj na stabilizację
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC));
    
    uint16_t reading = ADC;
    // Przelicz na temperaturę (przybliżony wzór dla ATmega328P)
    // 1.1V / 1024 * reading = voltage
    // Temp ≈ (voltage - 0.5) * 1000 / 3.24 + 25
    int temp = (reading * 1100UL / 1024 - 500) * 100 / 324 + 250;
    return (temp > 0) ? temp : 250;  // Min 25.0°C
}

// ============================================================================
// IMPLEMENTACJA FUNKCJI PUBLICZNYCH
// ============================================================================

/**
 * @brief Inicjalizuje system bezpieczeństwa
 */
void safety_init() {
    // Wyczyść status
    memset(&safety_status, 0, sizeof(SafetyStatus));
    
    // Odczytaj przyczynę ostatniego resetu
    uint8_t mcusr = read_mcusr();
    
    if (mcusr & _BV(WDRF)) {
        // Reset przez Watchdog
        wdt_reset_count = eeprom_read_reset_count();
        wdt_reset_count++;
        eeprom_write_reset_count(wdt_reset_count);
        
        safety_status.error_code = SAFE_ERROR_WDTO_RESET;
        safety_status.reset_count = wdt_reset_count;
        
        // Sprawdź czy nie przekroczono progu
        if (wdt_reset_count >= WDT_RESET_THRESHOLD) {
            safety_status.safe_state = SAFE_STATE_LOCKOUT;
        } else {
            safety_status.safe_state = SAFE_STATE_WARNING;
        }
    } else if (mcusr & _BV(PORF)) {
        // Power-on reset
        wdt_reset_count = 0;
        eeprom_write_reset_count(0);
        safety_status.safe_state = SAFE_STATE_OK;
    } else if (mcusr & _BV(EXTRF)) {
        // External reset
        safety_status.safe_state = SAFE_STATE_OK;
    }
    
    // Zainicjuj timery warstw
    for (int i = 0; i < 4; i++) {
        layer_timers[i] = millis();
    }
    
    // Konfiguruj Watchdog Timer
    wdt_enable(WDTO_2S);
    
    // Inicjuj ADC dla temperatury
    ADMUX = _BV(REFS0);  // AVCC as reference
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);  // Enable ADC, prescaler 128
    
    safety_status.uptime_ms = millis();
    
    // Feed all layers initially
    safety_feed(WDT_LAYER_MAIN);
    safety_feed(WDT_LAYER_NETWORK);
    safety_feed(WDT_LAYER_THERAPY);
    safety_feed(WDT_LAYER_COMMS);
}

/**
 * @brief Główna pętla systemu bezpieczeństwa (pseudo-wątek)
 * MUSI być wywoływana regularnie
 */
void safety_loop() {
    uint32_t now = millis();
    
    // Aktualizuj uptime
    safety_status.uptime_ms = now;
    
    // Mierz temperaturę co 1 sekundę
    static uint32_t last_temp_check = 0;
    if (now - last_temp_check >= 1000) {
        last_temp_check = now;
        safety_status.temperature_x10 = measure_temperature();
        
        // Sprawdź przegrzanie (>85°C = 850)
        if (safety_status.temperature_x10 > 850) {
            safety_status.error_code = SAFE_ERROR_OVERHEAT;
            safety_status.safe_state = SAFE_STATE_CRITICAL;
        } else if (safety_status.temperature_x10 > 700) {
            safety_status.safe_state = SAFE_STATE_WARNING;
        } else if (safety_status.safe_state != SAFE_STATE_LOCKOUT && 
                   safety_status.error_code != SAFE_ERROR_OVERHEAT) {
            safety_status.safe_state = SAFE_STATE_OK;
        }
    }
    
    // Sprawdź timery warstw Watchdog
    uint32_t max_allowed_delay = WDT_TIMEOUT_MS;
    
    for (int i = 0; i < 4; i++) {
        uint8_t layer_mask = (1 << i);
        if (safety_status.layer_status & layer_mask) {
            // Ta warstwa jest aktywna - sprawdź timeout
            if (now - layer_timers[i] > max_allowed_delay) {
                // Timeout warstwy!
                safety_status.error_code = SAFE_ERROR_WATCHDOG_FAIL;
                safety_status.safe_state = SAFE_STATE_CRITICAL;
                
                // Krytyczne - natychmiastowe działanie
                if (emergency_latch == false) {
                    emergency_latch = true;
                    // Wyłącz PWM itp. zostanie zrobione w głównym kodzie
                }
            }
        }
    }
    
    // Sprawdź czy nie jesteśmy w lockout
    if (safety_status.safe_state == SAFE_STATE_LOCKOUT) {
        // Blokada - nie pozwalaj na pracę
        return;
    }
    
    // Automatyczny feed Watchdog hardware jeśli wszystko OK
    if (safety_status.safe_state == SAFE_STATE_OK || 
        safety_status.safe_state == SAFE_STATE_WARNING) {
        wdt_reset();
    }
}

/**
 * @brief Feeduje Watchdog dla konkretnej warstwy
 * @param layer Mask bitowy warstwy (WDT_LAYER_*)
 */
void safety_feed(uint8_t layer) {
    uint32_t now = millis();
    
    // Znajdź indeks warstwy
    for (int i = 0; i < 4; i++) {
        if (layer & (1 << i)) {
            layer_timers[i] = now;
            safety_status.layer_status |= (1 << i);
        }
    }
}

/**
 * @brief Sprawdza czy system jest w bezpiecznym stanie
 * @return true jeśli bezpieczny, false jeśli błąd
 */
bool safety_is_safe() {
    return (safety_status.safe_state != SAFE_STATE_CRITICAL && 
            safety_status.safe_state != SAFE_STATE_LOCKOUT &&
            emergency_latch == false);
}

/**
 * @brief Pobiera kod błędu
 * @return Kod błędu
 */
uint8_t safety_get_error() {
    return safety_status.error_code;
}

/**
 * @brief Wyzwala procedurę emergency shutdown
 */
void safety_trigger_emergency() {
    emergency_latch = true;
    safety_status.safe_state = SAFE_STATE_CRITICAL;
    safety_status.error_code = SAFE_ERROR_FREQ_FAULT;  // Generic fault
    
    // Wyłącz wszystkie warstwy
    safety_status.layer_status = 0;
}

/**
 * @brief Pobiera pełny status bezpieczeństwa
 * @return Wskaźnik do struktury statusu
 */
SafetyStatus* safety_get_status() {
    return &safety_status;
}

/**
 * @brief Czyści blokadę emergency (tylko jeśli przyczyna usunięta)
 * @return true jeśli odblokowano, false jeśli nadal niebezpiecznie
 */
void safety_clear_latch() {
    // Można odblokować tylko jeśli temperatura OK i nie ma lockout
    if (safety_status.safe_state != SAFE_STATE_LOCKOUT &&
        safety_status.temperature_x10 < 700) {
        emergency_latch = false;
        if (safety_status.error_code == SAFE_ERROR_OVERHEAT &&
            safety_status.temperature_x10 < 700) {
            safety_status.error_code = SAFE_ERROR_NONE;
            safety_status.safe_state = SAFE_STATE_OK;
        }
    }
}

/**
 * @brief Pobiera temperaturę
 * @return Temperatura * 10 (np. 350 = 35.0°C)
 */
uint16_t safety_get_temperature() {
    return safety_status.temperature_x10;
}

/**
 * @brief Sprawdza stan termiczny
 * @return true jeśli temperatura w normie
 */
bool safety_check_thermal() {
    return (safety_status.temperature_x10 < 700);
}
