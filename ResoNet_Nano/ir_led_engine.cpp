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

/* 32-sample sine approximation in flash; avoids AVR libm float/sin cost. */
static const uint8_t modulation_sine[32] PROGMEM = {
    128, 153, 177, 199, 218, 234, 245, 253,
    255, 253, 245, 234, 218, 199, 177, 153,
    128, 103, 79, 57, 38, 22, 11, 3,
    0, 3, 11, 22, 38, 57, 79, 103
};

static uint8_t modulation_sine_value(uint16_t counter, uint16_t period) {
    if (period == 0U) return 128U;
    const uint8_t index = (uint8_t)(
        ((uint32_t)(counter % period) * 32UL) / period);
    return pgm_read_byte(&modulation_sine[index]);
}

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

/**
 * @brief Konfiguruje Timer2 dla carrier frequency 38kHz na pinie 3 (OC2B)
 * @param carrier_freq_hz Częstotliwość nośna (domyślnie 38000 Hz)
 */
static bool timer2_configure_carrier(uint16_t carrier_freq_hz) {
    // Timer2 is 8-bit. Prescaler 8 provides a representable 38 kHz carrier:
    // 16 MHz / (8 * 53) = 37.736 kHz (about -0.7%).
    if (carrier_freq_hz < 7800U || carrier_freq_hz > 1000000U) {
        return false;
    }

    const uint8_t saved_sreg = SREG;
    cli();

    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2 = 0;

    uint32_t counts = (F_CPU + (4UL * carrier_freq_hz)) /
                      (8UL * carrier_freq_hz);
    if (counts < 2UL) counts = 2UL;
    if (counts > 256UL) counts = 256UL;
    const uint8_t top = (uint8_t)(counts - 1UL);

    // Fast PWM mode 7, OCR2A as TOP, prescaler 8.
    OCR2A = top;
    OCR2B = top / 2U;
    TCCR2A = _BV(WGM20) | _BV(WGM21) | _BV(COM2B1);
    TCCR2B = _BV(WGM22) | _BV(CS21);

    SREG = saved_sreg;
    LOG_INFO("Timer2 carrier configured");
    return true;
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
    const uint8_t top = OCR2A;
    uint16_t compare = ((uint16_t)top * duty_cycle) / 100U;
    if (compare >= top) compare = top > 0U ? top - 1U : 0U;
    OCR2B = (uint8_t)compare;
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
                uint32_t mod_period_ms = 100000UL / ir_config.therapy_freq_hz_x100; // ms
                
                // Ensure update interval is at least 1ms to avoid zero division
                uint32_t update_interval = mod_period_ms / 100;
                if (update_interval < 1) update_interval = 1;
                
                if (now - last_am_update >= update_interval) {  // 100 kroków na okres
                    last_am_update = now;
                    
                    // Oblicz chwilową wartość modulacji (0-100%)
                    ir_status.modulation_counter++;
                    const uint8_t mod_factor = modulation_sine_value(
                        ir_status.modulation_counter, 1000U);
                    const uint8_t new_duty =
                        (uint16_t)base_duty_cycle * mod_factor / 255U;
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
                    const uint8_t wave = modulation_sine_value(
                        ir_status.modulation_counter, 2000U);
                    // Integer scale 900..1100 represents the original ±10% sine.
                    const uint16_t scale = 900U + ((uint16_t)wave * 200U) / 255U;
                    const uint16_t new_freq =
                        ((uint32_t)ir_config.therapy_freq_hz_x100 * scale) / 1000U;
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
    
    // Skonfiguruj carrier frequency; reject values Timer2 cannot represent.
    if (!timer2_configure_carrier(config->carrier_freq_hz)) {
        ir_status.is_running = false;
        ir_status.carrier_enabled = false;
        digitalWrite(IR_LED_PIN, LOW);
        LOG_ERROR("IR carrier frequency out of Timer2 range");
        return false;
    }

    // Aktualizuj status
    ir_status.current_therapy_freq_x100 = config->therapy_freq_hz_x100;
    ir_status.current_duty = config->duty_cycle;
    ir_status.current_modulation = config->modulation_type;
    ir_status.carrier_enabled = true;
    
    LOG_INFO("IR LED configured");
    
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
