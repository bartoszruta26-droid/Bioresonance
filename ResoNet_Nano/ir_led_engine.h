/**
 * @file ir_led_engine.h
 * @brief Silnik sterowania paskiem LED IR - Nagłówki
 * @version 1.0 (IR LED Strip Control with Carrier Frequency)
 * 
 * Generator sygnału IR z częstotliwością nośną 38kHz i modulacją terapeutyczną
 */

#ifndef IR_LED_ENGINE_H
#define IR_LED_ENGINE_H

#include <Arduino.h>
#include "types.h"

// ============================================================================
// KONFIGURACJA IR LED STRIP (Zgodne z hardware.md)
// ============================================================================

#define IR_LED_PIN            3   // Timer2 OC2B - Carrier 38kHz
#define IR_MODULATION_PIN     6   // Modulacja terapeutyczna AM/FM/Burst

#define IR_CARRIER_FREQ_HZ    38000  // Standardowa częstotliwość IR 38kHz
#define IR_MIN_THERAPY_HZ     1      // Minimalna częstotliwość terapii
#define IR_MAX_THERAPY_HZ     100    // Maksymalna częstotliwość terapii

// Typy modulacji IR (używamy z types.h)
// MODULATION_NONE = 0
// MODULATION_AM = 1
// MODULATION_FM = 2
// MODULATION_BURST = 3

// Struktura konfiguracji IR LED
typedef struct {
    uint16_t therapy_freq_hz_x100;  // Częstotliwość terapii * 100 (1-100 Hz)
    uint8_t duty_cycle;             // 0-100% intensywności
    uint8_t modulation_type;        // MODULATION_*
    uint16_t intensity_level;       // 0-4095
    uint32_t duration_ms;           // Czas trwania w ms
    uint16_t carrier_freq_hz;       // Częstotliwość nośna (38kHz domyślnie)
} IRLedConfig;

// Struktura stanu IR LED
typedef struct {
    bool is_running;
    uint16_t current_therapy_freq_x100;
    uint8_t current_duty;
    uint8_t current_modulation;
    uint32_t elapsed_ms;
    uint16_t modulation_counter;
    bool carrier_enabled;
} IRLedStatus;

// Funkcje publiczne
void ir_led_init();
void ir_led_loop();
bool ir_led_set_config(const IRLedConfig* config);
void ir_led_start();
void ir_led_stop();
IRLedStatus* ir_led_get_status();
bool ir_led_is_running();
void ir_led_update_intensity(uint16_t level);
void ir_led_enable_carrier(bool enable);

#endif // IR_LED_ENGINE_H
