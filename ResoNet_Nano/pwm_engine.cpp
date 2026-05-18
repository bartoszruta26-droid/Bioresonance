/**
 * @file pwm_engine.cpp
 * @brief Silnik generowania sygnałów PWM - Implementacja
 * @version 4.0 (Modular Architecture with Pseudo-threading)
 */

#include "pwm_engine.h"
#include "logging_system.h"
#include "event_system.h"

// ============================================================================
// ZMIENNE PRYWATNE
// ============================================================================

static PWMConfig pwm_config;
static PWMStatus pwm_status;

// Zmienne do modulacji
static uint32_t modulation_start_time = 0;
static uint16_t base_duty_cycle = 50;

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

/**
 * @brief Konfiguruje Timer1 dla PWM na pinie 9
 * @param frequency_hz_x100 Częstotliwość * 100
 * @param duty_cycle Duty cycle 0-100%
 */
static void timer1_configure_pwm(uint32_t frequency_hz_x100, uint8_t duty_cycle) {
    // Oblicz prescaler i ICR1
    uint16_t prescaler = 1;
    uint16_t icr_value;
    
    // F_CPU = 16MHz
    // frequency = F_CPU / (prescaler * ICR1)
    
    if (frequency_hz_x100 >= 500000) {  // >= 5 kHz
        prescaler = 1;
        icr_value = F_CPU / (frequency_hz_x100 / 100);
    } else if (frequency_hz_x100 >= 62500) {  // >= 625 Hz
        prescaler = 8;
        icr_value = F_CPU / (8UL * (frequency_hz_x100 / 100));
    } else if (frequency_hz_x100 >= 7812) {  // >= 78.12 Hz
        prescaler = 64;
        icr_value = F_CPU / (64UL * (frequency_hz_x100 / 100));
    } else if (frequency_hz_x100 >= 1953) {  // >= 19.53 Hz
        prescaler = 256;
        icr_value = F_CPU / (256UL * (frequency_hz_x100 / 100));
    } else {  // < 19.53 Hz
        prescaler = 1024;
        icr_value = F_CPU / (1024UL * (frequency_hz_x100 / 100));
    }
    
    // Ogranicz ICR1 do rozsądnych wartości
    if (icr_value < 100) icr_value = 100;
    if (icr_value > 65535) icr_value = 65535;
    
    // Oblicz OCR1A z duty cycle
    uint16_t ocr_value = (uint32_t)icr_value * duty_cycle / 100;
    
    // Konfiguruj Timer1
    cli();
    
    // Wyłącz Timer1 podczas konfiguracji
    TCCR1A = 0;
    TCCR1B = 0;
    
    // Ustaw tryb Fast PWM z ICR1 jako TOP (tryb 14)
    TCCR1A = _BV(WGM11);
    TCCR1B = _BV(WGM12) | _BV(WGM13);
    
    // Ustaw prescaler
    switch (prescaler) {
        case 1:   TCCR1B |= _BV(CS10); break;
        case 8:   TCCR1B |= _BV(CS11); break;
        case 64:  TCCR1B |= _BV(CS11) | _BV(CS10); break;
        case 256: TCCR1B |= _BV(CS12); break;
        case 1024: TCCR1B |= _BV(CS12) | _BV(CS10); break;
    }
    
    // Ustaw ICR1 i OCR1A
    ICR1 = icr_value;
    OCR1A = ocr_value;
    
    // Włącz wyjście OC1A w trybie non-inverting
    TCCR1A |= _BV(COM1A1);
    
    sei();
}

/**
 * @brief Aktualizuje duty cycle
 */
static void timer1_update_duty(uint8_t duty_cycle) {
    uint16_t duty_value = (uint32_t)ICR1 * duty_cycle / 100;
    OCR1A = duty_value;
}

// ============================================================================
// IMPLEMENTACJA FUNKCJI PUBLICZNYCH
// ============================================================================

/**
 * @brief Inicjalizuje silnik PWM
 */
void pwm_init() {
    // Skonfiguruj pin wyjściowy
    pinMode(PWM_OUTPUT_PIN, OUTPUT);
    digitalWrite(PWM_OUTPUT_PIN, LOW);
    
    // Wyzeruj konfigurację
    memset(&pwm_config, 0, sizeof(PWMConfig));
    memset(&pwm_status, 0, sizeof(PWMStatus));
    
    // Domyślna konfiguracja
    pwm_config.duty_cycle = 50;
    pwm_config.intensity_level = 2048;
    
    LOG_INFO("PWM engine initialized");
}

/**
 * @brief Główna pętla silnika PWM (pseudo-wątek)
 * Obsługuje modulacje i aktualizację parametrów
 */
void pwm_loop() {
    if (!pwm_status.is_running) {
        return;
    }
    
    uint32_t now = millis();
    
    // Aktualizuj czas trwania
    pwm_status.elapsed_ms = now - modulation_start_time;
    
    // Sprawdź czy terapia zakończona
    if (pwm_config.duration_ms > 0 && 
        pwm_status.elapsed_ms >= pwm_config.duration_ms) {
        pwm_stop();
        EVENT_INFO(EVENT_THERAPY_COMPLETE, "Therapy session completed");
        return;
    }
    
    // Obsługa modulacji
    switch (pwm_config.modulation_type) {
        case MODULATION_NONE:
            // Brak modulacji - stały sygnał
            break;
            
        case MODULATION_AM:
            // Modulacja amplitudy (sinusoidalna, 1 Hz)
            {
                static uint32_t last_am_update = 0;
                if (now - last_am_update >= 10) {  // Co 10ms
                    last_am_update = now;
                    
                    // Oblicz chwilową wartość modulacji (0-100%)
                    // Prosta aproksymacja sinusa
                    pwm_status.modulation_counter++;
                    float phase = (pwm_status.modulation_counter % 1000) / 1000.0 * 2.0 * PI;
                    float mod_factor = 0.5 + 0.5 * sin(phase);  // 0.0 do 1.0
                    
                    uint8_t new_duty = (uint8_t)(base_duty_cycle * mod_factor);
                    timer1_update_duty(new_duty);
                }
            }
            break;
            
        case MODULATION_FM:
            // Modulacja częstotliwości (±10%, 0.5 Hz)
            {
                static uint32_t last_fm_update = 0;
                if (now - last_fm_update >= 50) {  // Co 50ms
                    last_fm_update = now;
                    
                    pwm_status.modulation_counter++;
                    float phase = (pwm_status.modulation_counter % 2000) / 2000.0 * 2.0 * PI;
                    float freq_factor = 1.0 + 0.1 * sin(phase);  // ±10%
                    
                    uint32_t new_freq = (uint32_t)(pwm_config.frequency_hz_x100 * freq_factor);
                    timer1_configure_pwm(new_freq, pwm_status.current_duty);
                    pwm_status.current_freq_x100 = new_freq;
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
                    
                    if (burst_state) {
                        timer1_update_duty(base_duty_cycle);
                    } else {
                        timer1_update_duty(0);
                    }
                }
            }
            break;
    }
    
    // Feed watchdog warstwy terapii
    // Będzie wywołane w głównym pliku .ino
}

/**
 * @brief Ustawia konfigurację PWM
 * @param config Wskaźnik do struktury konfiguracji
 * @return true jeśli sukces
 */
bool pwm_set_config(const PWMConfig* config) {
    if (!config) return false;
    
    // Walidacja parametrów (zgodnie z types.h)
    if (config->frequency_hz_x100 < PWM_MIN_FREQUENCY_HZ_X100 ||
        config->frequency_hz_x100 > PWM_MAX_FREQUENCY_HZ_X100) {
        LOG_ERROR("Frequency out of range");
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
    memcpy(&pwm_config, config, sizeof(PWMConfig));
    
    // Zapisz bazowy duty cycle
    base_duty_cycle = config->duty_cycle;
    
    // Skonfiguruj timer
    timer1_configure_pwm(config->frequency_hz_x100, config->duty_cycle);
    
    // Aktualizuj status
    pwm_status.current_freq_x100 = config->frequency_hz_x100;
    pwm_status.current_duty = config->duty_cycle;
    pwm_status.current_modulation = config->modulation_type;
    
    LOG_INFO_F("PWM configured: %lu.%02lu Hz, duty %u%%", 
               config->frequency_hz_x100 / 100,
               config->frequency_hz_x100 % 100,
               config->duty_cycle);
    
    return true;
}

/**
 * @brief Uruchamia generator PWM
 */
void pwm_start() {
    if (pwm_status.is_running) {
        return;
    }
    
    modulation_start_time = millis();
    pwm_status.elapsed_ms = 0;
    pwm_status.modulation_counter = 0;
    pwm_status.is_running = true;
    
    // Przywróć duty cycle po zatrzymaniu
    timer1_update_duty(base_duty_cycle);
    
    EVENT_INFO(EVENT_THERAPY_START, "Therapy started");
    LOG_INFO("PWM output enabled");
}

/**
 * @brief Zatrzymuje generator PWM
 */
void pwm_stop() {
    if (!pwm_status.is_running) {
        return;
    }
    
    // Wyłącz wyjście PWM
    timer1_update_duty(0);
    
    pwm_status.is_running = false;
    
    EVENT_INFO(EVENT_THERAPY_STOP, "Therapy stopped");
    LOG_INFO("PWM output disabled");
}

/**
 * @brief Pobiera status silnika PWM
 * @return Wskaźnik do struktury statusu
 */
PWMStatus* pwm_get_status() {
    return &pwm_status;
}

/**
 * @brief Sprawdza czy PWM jest aktywny
 * @return true jeśli działa
 */
bool pwm_is_running() {
    return pwm_status.is_running;
}

/**
 * @brief Pobiera aktualną częstotliwość
 * @return Częstotliwość * 100
 */
uint32_t pwm_get_frequency() {
    return pwm_status.current_freq_x100;
}

/**
 * @brief Aktualizuje poziom intensywności
 * @param level Wartość 0-4095
 */
void pwm_update_intensity(uint16_t level) {
    if (level > 4095) level = 4095;
    
    pwm_config.intensity_level = level;
    
    // Intensywność może wpływać na duty cycle w przyszłych wersjach
    // Na razie tylko zapisujemy wartość
}
