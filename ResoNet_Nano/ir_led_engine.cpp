/**
 * @file ir_led_engine.cpp
 * @brief Silnik sterowania paskiem LED IR - Implementacja
 * @version 1.0 (IR LED Strip Control with Carrier Frequency)
 * 
 * Generuje sygnał IR z częstotliwością nośną 38kHz modulowany terapeutycznie
 */

#include "ir_led_engine.h"
#include "logging_system.h"
#include "event_system.h"

// ============================================================================
// ZMIENNE PRYWATNE
// ============================================================================

static IRLedConfig ir_config;
static IRLedStatus ir_status;

// Zmienne do modulacji
static uint32_t modulation_start_time = 0;
static uint8_t base_duty_cycle = 50;

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

/**
 * @brief Konfiguruje Timer2 dla carrier frequency 38kHz na pinie 5
 * @param carrier_freq_hz Częstotliwość nośna (domyślnie 38000 Hz)
 */
static void timer2_configure_carrier(uint16_t carrier_freq_hz) {
    // Pin 5 jest podłączony do OC0B (Timer0) lub można użyć Timer2
    // Dla 38kHz używamy Timer2 w trybie Fast PWM
    
    cli();
    
    // Wyłącz Timer2 podczas konfiguracji
    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2 = 0;
    
    // Tryb Fast PWM, TOP = OCR2A (tryb 7)
    // f_pwm = f_clk / (N * (1 + OCR2A))
    // Dla 38kHz: OCR2A = (16000000 / 38000) - 1 ≈ 420
    
    uint16_t ocr2a_value = (F_CPU / carrier_freq_hz) - 1;
    if (ocr2a_value > 255) ocr2a_value = 255;
    if (ocr2a_value < 1) ocr2a_value = 1;
    
    // Ustaw tryb Fast PWM z OCR2A jako TOP
    TCCR2A = _BV(WGM20) | _BV(WGM21);  // Mode 7: Fast PWM, TOP=OCR2A
    TCCR2B = _BV(CS20);                 // Prescaler = 1 (no division)
    
    // Ustaw TOP
    OCR2A = ocr2a_value;
    
    // Duty cycle 50% dla carrier (modulowane przez AM)
    OCR2B = ocr2a_value / 2;
    
    // Włącz wyjście OC2B na pinie 3 (Arduino) lub użyj pinu 5 z Timer0
    // Dla pinu 5 (OC0B) musimy użyć Timer0
    TCCR2A |= _BV(COM2B1);  // Non-inverting mode on OC2B
    
    sei();
    
    LOG_INFO_F("Timer2 configured for %u Hz carrier", carrier_freq_hz);
}

/**
 * @brief Alternatywna konfiguracja dla pinu 5 (Timer0)
 * @param carrier_freq_hz Częstotliwość nośna
 */
static void timer0_configure_carrier_5pin(uint16_t carrier_freq_hz) {
    // UWAGA: Timer0 jest używany przez Arduino millis()/delay()
    // Modyfikacja może wpłynąć na timing systemu!
    // Zalecane użycie pinu 3 (Timer2) lub zmiana podejścia
    
    cli();
    
    // Zachowaj oryginalne ustawienia dla millis()
    // Użyjemy metody software PWM dla pinu 5 jeśli konieczne
    
    sei();
}

/**
 * @brief Aktualizuje duty cycle carrier frequency
 * @param duty_cycle Duty cycle 0-100%
 */
static void timer2_update_duty(uint8_t duty_cycle) {
    uint8_t ocr_value = (uint16_t)OCR2A * duty_cycle / 100;
    OCR2B = ocr_value;
}

/**
 * @brief Włącza/wyłącza carrier frequency
 * @param enable true = włącz, false = wyłącz
 */
static void timer2_enable_carrier(bool enable) {
    if (enable) {
        TCCR2A |= _BV(COM2B1);  // Enable output
    } else {
        TCCR2A &= ~(_BV(COM2B1));  // Disable output
        digitalWrite(IR_LED_PIN, LOW);
    }
}

// ============================================================================
// IMPLEMENTACJA FUNKCJI PUBLICZNYCH
// ============================================================================

/**
 * @brief Inicjalizuje silnik IR LED
 */
void ir_led_init() {
    // Skonfiguruj piny wyjściowe
    pinMode(IR_LED_PIN, OUTPUT);
    pinMode(IR_MODULATION_PIN, OUTPUT);
    digitalWrite(IR_LED_PIN, LOW);
    digitalWrite(IR_MODULATION_PIN, LOW);
    
    // Wyzeruj konfigurację
    memset(&ir_config, 0, sizeof(IRLedConfig));
    memset(&ir_status, 0, sizeof(IRLedStatus));
    
    // Domyślna konfiguracja
    ir_config.duty_cycle = 50;
    ir_config.intensity_level = 2048;
    ir_config.carrier_freq_hz = IR_CARRIER_FREQ_HZ;
    
    LOG_INFO("IR LED engine initialized");
}

/**
 * @brief Główna pętla silnika IR LED (pseudo-wątek)
 * Obsługuje modulacje i aktualizację parametrów
 */
void ir_led_loop() {
    if (!ir_status.is_running) {
        return;
    }
    
    uint32_t now = millis();
    
    // Aktualizuj czas trwania
    ir_status.elapsed_ms = now - modulation_start_time;
    
    // Sprawdź czy terapia zakończona
    if (ir_config.duration_ms > 0 && 
        ir_status.elapsed_ms >= ir_config.duration_ms) {
        ir_led_stop();
        EVENT_INFO(EVENT_THERAPY_COMPLETE, "IR therapy session completed");
        return;
    }
    
    // Obsługa modulacji terapeutycznej
    switch (ir_config.modulation_type) {
        case MODULATION_NONE:
            // Brak modulacji - stały sygnał carrier
            break;
            
        case MODULATION_AM:
            // Modulacja amplitudy (1-100 Hz)
            {
                static uint32_t last_am_update = 0;
                uint16_t mod_period_ms = 100000 / ir_config.therapy_freq_hz_x100; // ms
                
                if (now - last_am_update >= mod_period_ms / 100) {  // 100 kroków na okres
                    last_am_update = now;
                    
                    // Oblicz chwilową wartość modulacji (0-100%)
                    ir_status.modulation_counter++;
                    float phase = (ir_status.modulation_counter % 1000) / 1000.0 * 2.0 * PI;
                    float mod_factor = 0.5 + 0.5 * sin(phase);  // 0.0 do 1.0
                    
                    uint8_t new_duty = (uint8_t)(base_duty_cycle * mod_factor);
                    timer2_update_duty(new_duty);
                }
            }
            break;
            
        case MODULATION_FM:
            // Modulacja częstotliwości terapii (±10%)
            {
                static uint32_t last_fm_update = 0;
                if (now - last_fm_update >= 50) {  // Co 50ms
                    last_fm_update = now;
                    
                    ir_status.modulation_counter++;
                    float phase = (ir_status.modulation_counter % 2000) / 2000.0 * 2.0 * PI;
                    float freq_factor = 1.0 + 0.1 * sin(phase);  // ±10%
                    
                    uint16_t new_freq = (uint16_t)(ir_config.therapy_freq_hz_x100 * freq_factor);
                    ir_status.current_therapy_freq_x100 = new_freq;
                    // FM nie zmienia carrier frequency, tylko tempo modulacji
                }
            }
            break;
            
        case MODULATION_BURST:
            // Sygnał burst (włącz/wyłącz co 500ms)
            {
                static bool burst_state = true;
                static uint32_t last_burst_change = 0;
                
                if (now - last_burst_change >= 500) {
                    last_burst_change = now;
                    burst_state = !burst_state;
                    
                    ir_led_enable_carrier(burst_state);
                    ir_status.carrier_enabled = burst_state;
                }
            }
            break;
    }
}

/**
 * @brief Ustawia konfigurację IR LED
 * @param config Wskaźnik do struktury konfiguracji
 * @return true jeśli sukces
 */
bool ir_led_set_config(const IRLedConfig* config) {
    if (!config) return false;
    
    // Walidacja parametrów
    if (config->therapy_freq_hz_x100 < IR_MIN_THERAPY_HZ * 100 ||
        config->therapy_freq_hz_x100 > IR_MAX_THERAPY_HZ * 100) {
        LOG_ERROR("Therapy frequency out of range (1-100 Hz)");
        return false;
    }
    
    if (config->duty_cycle > 100 || config->duty_cycle == 0) {
        LOG_ERROR("Duty cycle invalid (must be 1-100%)");
        return false;
    }
    
    if (config->modulation_type > MODULATION_BURST) {
        LOG_ERROR("Invalid modulation type");
        return false;
    }
    
    if (config->intensity_level > 4095) {
        LOG_ERROR("Intensity level out of range");
        return false;
    }
    
    // Zapisz konfigurację
    memcpy(&ir_config, config, sizeof(IRLedConfig));
    
    // Zapisz bazowy duty cycle
    base_duty_cycle = config->duty_cycle;
    
    // Skonfiguruj carrier frequency
    timer2_configure_carrier(config->carrier_freq_hz);
    
    // Aktualizuj status
    ir_status.current_therapy_freq_x100 = config->therapy_freq_hz_x100;
    ir_status.current_duty = config->duty_cycle;
    ir_status.current_modulation = config->modulation_type;
    ir_status.carrier_enabled = true;
    
    LOG_INFO_F("IR LED configured: therapy=%lu.%02lu Hz, carrier=%u Hz, duty %u%%", 
               config->therapy_freq_hz_x100 / 100,
               config->therapy_freq_hz_x100 % 100,
               config->carrier_freq_hz,
               config->duty_cycle);
    
    return true;
}

/**
 * @brief Uruchamia generator IR LED
 */
void ir_led_start() {
    if (ir_status.is_running) {
        return;
    }
    
    modulation_start_time = millis();
    ir_status.elapsed_ms = 0;
    ir_status.modulation_counter = 0;
    ir_status.is_running = true;
    ir_status.carrier_enabled = true;
    
    // Włącz carrier
    timer2_enable_carrier(true);
    
    EVENT_INFO(EVENT_THERAPY_START, "IR therapy started");
    LOG_INFO("IR LED output enabled");
}

/**
 * @brief Zatrzymuje generator IR LED
 */
void ir_led_stop() {
    if (!ir_status.is_running) {
        return;
    }
    
    // Wyłącz carrier
    timer2_enable_carrier(false);
    
    ir_status.is_running = false;
    ir_status.carrier_enabled = false;
    
    EVENT_INFO(EVENT_THERAPY_STOP, "IR therapy stopped");
    LOG_INFO("IR LED output disabled");
}

/**
 * @brief Pobiera status silnika IR LED
 * @return Wskaźnik do struktury statusu
 */
IRLedStatus* ir_led_get_status() {
    return &ir_status;
}

/**
 * @brief Sprawdza czy IR LED jest aktywny
 * @return true jeśli działa
 */
bool ir_led_is_running() {
    return ir_status.is_running;
}

/**
 * @brief Aktualizuje poziom intensywności
 * @param level Wartość 0-4095
 */
void ir_led_update_intensity(uint16_t level) {
    if (level > 4095) level = 4095;
    
    ir_config.intensity_level = level;
    
    // Intensywność wpływa na duty cycle
    uint8_t new_duty = (uint8_t)((uint32_t)base_duty_cycle * level / 4095);
    timer2_update_duty(new_duty);
}

/**
 * @brief Włącza/wyłącza carrier frequency
 * @param enable true = włącz, false = wyłącz
 */
void ir_led_enable_carrier(bool enable) {
    timer2_enable_carrier(enable);
    ir_status.carrier_enabled = enable;
}
