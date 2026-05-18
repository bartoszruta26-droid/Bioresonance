/**
 * @file pwm_engine.h
 * @brief Silnik generowania sygnałów PWM - Nagłówki
 * @version 4.0 (Modular Architecture with Pseudo-threading)
 * 
 * Generator XV-LPWM (0.1 Hz - 500 kHz, rozdzielczość 0.01 Hz)
 */

#ifndef PWM_ENGINE_H
#define PWM_ENGINE_H

#include <Arduino.h>

// ============================================================================
// KONFIGURACJA PWM
// ============================================================================

#define PWM_OUTPUT_PIN 9        // Timer1 Channel A (OC1A)
#define PWM_MIN_FREQUENCY_HZ_X100 10      // 0.1 Hz
#define PWM_MAX_FREQUENCY_HZ_X100 5000000 // 500 kHz

// Typy modulacji
#define MODULATION_NONE     0
#define MODULATION_AM       1
#define MODULATION_FM       2
#define MODULATION_BURST    3

// Struktura konfiguracji PWM
typedef struct {
    uint32_t frequency_hz_x100;   // Częstotliwość * 100
    uint8_t duty_cycle;           // 0-100%
    uint8_t modulation_type;      // MODULATION_*
    uint16_t intensity_level;     // 0-4095
    uint32_t duration_ms;         // Czas trwania w ms
} PWMConfig;

// Struktura stanu PWM
typedef struct {
    bool is_running;
    uint32_t current_freq_x100;
    uint8_t current_duty;
    uint8_t current_modulation;
    uint32_t elapsed_ms;
    uint16_t modulation_counter;
} PWMStatus;

// Funkcje publiczne
void pwm_init();
void pwm_loop();
bool pwm_set_config(const PWMConfig* config);
void pwm_start();
void pwm_stop();
PWMStatus* pwm_get_status();
bool pwm_is_running();
uint32_t pwm_get_frequency();
void pwm_update_intensity(uint16_t level);

#endif // PWM_ENGINE_H
