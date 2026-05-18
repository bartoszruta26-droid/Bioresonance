/**
 * @file device_detector.cpp
 * @brief Implementacja systemu wykrywania efektorów i sensorów
 * @version 1.1 (Rozszerzona o debug, verbose, CLI, event handling)
 */

#include "device_detector.h"
#include "logging_system.h"
#include "pwm_engine.h"
#include "event_system.h"
#include <stdarg.h>   // Dla va_list, va_start, va_end
#include <string.h>   // Dla strcmp, strncmp, memcpy, strncpy
#include <avr/pgmspace.h>  // Dla PROGMEM, pgm_read_word
#include "ir_led_engine.h"

// ============================================================================
// ZMIENNE GLOBALNE
// ============================================================================

static DeviceSystemState_t g_device_state;
static EffectorType_t g_current_effector = EFFECTOR_NONE;
static BioSensorStatus_t g_bio_status;

// Konfiguracja detekcji - może być nadpisana przez CLI
static DetectionConfig_t s_detection_config = {
    .verbose_mode = DETECTION_VERBOSE_MODE,
    .debug_enabled = DETECTION_DEBUG_ENABLED,
    .force_calibration = false,
    .detection_interval = 5,  // Co 5 sekund
    .quiet_mode = false
};

// Liczniki statystyk
static uint32_t s_error_count = 0;
static uint32_t s_detection_events = 0;

// Tablica opisów błędów w pamięci FLASH
static const char* const s_error_strings[] PROGMEM = {
    "OK",                              // DETECT_OK
    "Urządzenie nie wykryte",          // DETECT_ERR_NOT_FOUND
    "Zwarcie wykryte",                 // DETECT_ERR_SHORT_CIRCUIT
    "Przerwa w obwodzie",              // DETECT_ERR_OPEN_CIRCUIT
    "Przegrzanie urządzenia",          // DETECT_ERR_OVERTEMP
    "Nieprawidłowa impedancja",        // DETECT_ERR_INVALID_IMP
    "Błąd komunikacji",                // DETECT_ERR_COMM_FAIL
    "Timeout operacji",                // DETECT_ERR_TIMEOUT
    "Błąd konfiguracji"                // DETECT_ERR_CONFIG
};

// ============================================================================
// FUNKCJE POMOCNICZE - DEBUG & VERBOSE LOGGING
// ============================================================================

/**
 * @brief Logowanie z formatowaniem i obsługą poziomów verbose
 */
void detection_log_printf(uint8_t level, const char* format, ...) {
    // Sprawdź czy tryb quiet pozwala na ten poziom logowania
    if (s_detection_config.quiet_mode && level < LOG_ERROR) {
        return;
    }
    
    // Sprawdź tryb verbose dla DEBUG/VERBOSE
    if (!s_detection_config.verbose_mode && (level == LOG_DEBUG || level == LOG_VERBOSE)) {
        return;
    }
    
    if (!s_detection_config.debug_enabled && level == LOG_DEBUG) {
        return;
    }
    
    char buffer[LOG_MESSAGE_MAX_LEN];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    log_add_entry(level, 0, buffer);
}

/**
 * @brief Makro wewnętrzne dla debug logów
 */
#define DBG_LOG(msg) do { \
    if (s_detection_config.debug_enabled) { \
        detection_log_printf(LOG_DEBUG, msg); \
    } \
} while(0)

#define DBG_LOG_F(fmt, ...) do { \
    if (s_detection_config.debug_enabled) { \
        detection_log_printf(LOG_DEBUG, fmt, ##__VA_ARGS__); \
    } \
} while(0)

/**
 * @brief Obsługa zdarzeń detekcji
 */
void detection_handle_event(EventType_t event_type, EffectorType_t effector) {
    s_detection_events++;
    
    // Wyślij zdarzenie do systemu eventów
    char event_desc[64];
    snprintf(event_desc, sizeof(event_desc), "Detector: %d", effector);
    event_push(event_type, EVENT_SEVERITY_INFO, (uint32_t)effector, event_desc);
    
    // Loguj zdarzenie jeśli verbose
    if (s_detection_config.verbose_mode) {
        DBG_LOG_F("Event handled: 0x%04X for effector %d", event_type, effector);
    }
}

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

/**
 * @brief Oblicz temperaturę z odczytu NTC
 */
static float read_ntc_temperature(int pin) {
    int raw = analogRead(pin);
    // Steinhart-Hart approximation dla NTC 10k
    float resistance = 10000.0f * (float)raw / (1024.0f - (float)raw);
    float temperature = 1.0f / (0.003354f + 0.0003354f * log(resistance / 10000.0f));
    return temperature - 273.15f;  // Konwersja na °C
}

/**
 * @brief Krótkie opóźnienie w mikrosekundach
 */
static void delay_us(uint32_t us) {
    delayMicroseconds(us);
}

// ============================================================================
// INICJALIZACJA Z OBSŁUGĄ CLI
// ============================================================================

/**
 * @brief Inicjalizacja systemu detekcji z konfiguracją
 */
void device_detector_init(DetectionConfig_t* config) {
    // Jeśli podano konfigurację, użyj jej; w przeciwnym razie użyj domyślnej
    if (config != NULL) {
        memcpy(&s_detection_config, config, sizeof(DetectionConfig_t));
    }
    
    DBG_LOG("Device detector initialization started");
    
    // Zerowanie stanu
    memset(&g_device_state, 0, sizeof(DeviceSystemState_t));
    memset(&g_bio_status, 0, sizeof(BioSensorStatus_t));
    
    // Konfiguracja pinów cyfrowych
    pinMode(PIN_HELMHOLTZ_DETECT, INPUT_PULLUP);
    pinMode(PIN_OTIC_ENABLE, OUTPUT);
    pinMode(PIN_ELECTRODE_ENABLE, OUTPUT);
    pinMode(PIN_WRAP_ENABLE, OUTPUT);
    pinMode(PIN_BIO_DETECT, INPUT_PULLUP);
    pinMode(PIN_PPG_INT, INPUT);
    pinMode(PPG_SPI_CS, OUTPUT);
    pinMode(PIN_IR_DETECT, INPUT_PULLUP);
    
    // Ustawienie stanów początkowych - wszystkie wyłączone dla bezpieczeństwa
    digitalWrite(PIN_OTIC_ENABLE, LOW);
    digitalWrite(PIN_ELECTRODE_ENABLE, LOW);
    digitalWrite(PIN_WRAP_ENABLE, LOW);
    digitalWrite(PPG_SPI_CS, HIGH);
    digitalWrite(PIN_IR_DETECT, HIGH);
    
    // Inicjalizacja I2C dla biofeedback
    Wire.begin();
    
    // Inicjalizacja SPI dla PPG
    SPI.begin();
    
    g_device_state.biofeedbackEnabled = false;
    g_device_state.detectionCount = 0;
    g_device_state.errorCount = 0;
    
    // Reset liczników błędów
    s_error_count = 0;
    s_detection_events = 0;
    
    // Loguj tryb pracy
    if (s_detection_config.verbose_mode) {
        DBG_LOG("Verbose mode ENABLED");
    }
    if (s_detection_config.quiet_mode) {
        LOG_INFO("Quiet mode enabled - errors only");
    }
    
    LOG_INFO("Device detector initialized successfully");
    DBG_LOG_F("Detection interval: %d seconds", s_detection_config.detection_interval);
}

/**
 * @brief Parsowanie argumentów CLI (symulacja dla Arduino)
 * @param argc Liczba argumentów
 * @param argv Tablica stringów z argumentami
 * @return true jeśli parsowanie udane
 */
bool detection_parse_cli_args(int argc, char** argv) {
    if (argc == 0 || argv == NULL) {
        return false;  // Brak argumentów - użyj domyślnych
    }
    
    DBG_LOG_F("Parsing %d CLI arguments", argc);
    
    for (int i = 0; i < argc; i++) {
        // Obsługa flag --verbose / -v
        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            s_detection_config.verbose_mode = true;
            LOG_INFO("CLI: Verbose mode enabled");
        }
        // Obsługa flag --debug / -d
        else if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
            s_detection_config.debug_enabled = true;
            LOG_INFO("CLI: Debug mode enabled");
        }
        // Obsługa flag --quiet / -q
        else if (strcmp(argv[i], "--quiet") == 0 || strcmp(argv[i], "-q") == 0) {
            s_detection_config.quiet_mode = true;
            LOG_INFO("CLI: Quiet mode enabled");
        }
        // Obsługa --interval=N
        else if (strncmp(argv[i], "--interval=", 11) == 0) {
            int interval = atoi(argv[i] + 11);
            if (interval > 0 && interval <= 60) {
                s_detection_config.detection_interval = (uint8_t)interval;
                LOG_INFO_F("CLI: Detection interval set to %d seconds", interval);
            }
        }
        // Obsługa --calibrate / -c
        else if (strcmp(argv[i], "--calibrate") == 0 || strcmp(argv[i], "-c") == 0) {
            s_detection_config.force_calibration = true;
            LOG_INFO("CLI: Force calibration enabled");
        }
        else {
            LOG_WARNING_F("CLI: Unknown argument '%s'", argv[i]);
        }
    }
    
    return true;
}

// ============================================================================
// DETEKCJA EFEKTORÓW
// ============================================================================

bool detect_helmholtz_coil() {
    LOG_DEBUG("Detecting Helmholtz coil...");
    
    // Testowy impuls PWM
    analogWrite(PIN_PWM_OUTPUT, 128);  // 50% duty cycle
    delay_us(1000);
    
    // Pomiar prądu
    int senseValue = analogRead(PIN_HELMHOLTZ_SENSE);
    
    // Zatrzymaj PWM
    analogWrite(PIN_PWM_OUTPUT, 0);
    
    if (senseValue < HELMHOLTZ_CONNECTED_THRESHOLD) {
        LOG_INFO_F("Helmholtz coil detected (ADC=%d)", senseValue);
        g_device_state.effector.type = EFFECTOR_HELMHOLTZ;
        g_device_state.effector.connected = true;
        g_device_state.effector.impedance = senseValue;
        g_device_state.effector.quality = map(senseValue, 0, HELMHOLTZ_CONNECTED_THRESHOLD, 100, 50);
        return true;
    } else {
        LOG_WARNING("Helmholtz coil not detected");
        return false;
    }
}

bool detect_otic_applicator() {
    LOG_DEBUG("Detecting Otic applicator...");
    
    // Włącz driver
    digitalWrite(PIN_OTIC_ENABLE, HIGH);
    
    // Generuj testowy sygnał 10 kHz
    uint32_t old_freq = pwm_get_frequency();
    // Tymczasowa zmiana częstotliwości (wymaga implementacji w pwm_engine)
    analogWrite(PIN_PWM_OUTPUT, 64);  // 25% duty cycle
    delay_us(5000);
    
    // Pomiar impedancji (średnia z 10 próbek)
    uint32_t sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += analogRead(PIN_OTIC_IMPEDANCE);
        delay_us(100);
    }
    int impedanceValue = sum / 10;
    
    // Zatrzymaj PWM i wyłącz driver
    analogWrite(PIN_PWM_OUTPUT, 0);
    digitalWrite(PIN_OTIC_ENABLE, LOW);
    
    if (impedanceValue >= OTIC_CONNECTED_MIN && impedanceValue <= OTIC_CONNECTED_MAX) {
        LOG_INFO_F("Otic applicator detected (Z=%d)", impedanceValue);
        g_device_state.effector.type = EFFECTOR_OTIC;
        g_device_state.effector.connected = true;
        g_device_state.effector.impedance = impedanceValue;
        g_device_state.effector.quality = 80;
        return true;
    } else if (impedanceValue > OTIC_OPEN_CIRCUIT) {
        LOG_WARNING("Otic applicator not connected");
        return false;
    } else {
        LOG_ERROR_F("Otic applicator short circuit! (Z=%d)", impedanceValue);
        g_device_state.errorCount++;
        return false;
    }
}

ContactQuality_t detect_electrodes() {
    LOG_DEBUG("Detecting Contact electrodes...");
    
    // Włącz driver
    digitalWrite(PIN_ELECTRODE_ENABLE, HIGH);
    
    // Testowy impuls bipolarny (1ms)
    analogWrite(PIN_PWM_OUTPUT, 32);
    delay_us(500);
    analogWrite(PIN_PWM_OUTPUT, 0);
    
    // Pomiar impedancji (średnia z 16 próbek)
    uint32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogRead(PIN_ELECTRODE_IMPEDANCE);
        delay_us(50);
    }
    int impedanceValue = sum / 16;
    
    // Zatrzymaj PWM i wyłącz driver
    analogWrite(PIN_PWM_OUTPUT, 0);
    digitalWrite(PIN_ELECTRODE_ENABLE, LOW);
    
    g_device_state.effector.impedance = impedanceValue;
    
    // Ocena jakości
    if (impedanceValue < 100) {
        LOG_ERROR_F("Electrode SHORT CIRCUIT! (Z=%d)", impedanceValue);
        g_device_state.errorCount++;
        return CONTACT_SHORT;
    } else if (impedanceValue >= ELECTRODE_CONNECTED_MIN && impedanceValue <= ELECTRODE_OPTIMAL_MIN) {
        LOG_INFO_F("Good electrode contact (Z=%d)", impedanceValue);
        g_device_state.effector.type = EFFECTOR_CONTACT;
        g_device_state.effector.connected = true;
        g_device_state.effector.quality = 70;
        return CONTACT_GOOD;
    } else if (impedanceValue > ELECTRODE_OPTIMAL_MIN && impedanceValue <= ELECTRODE_OPTIMAL_MAX) {
        LOG_INFO_F("Excellent electrode contact (Z=%d)", impedanceValue);
        g_device_state.effector.type = EFFECTOR_CONTACT;
        g_device_state.effector.connected = true;
        g_device_state.effector.quality = 100;
        return CONTACT_EXCELLENT;
    } else if (impedanceValue > ELECTRODE_OPTIMAL_MAX && impedanceValue <= ELECTRODE_CONNECTED_MAX) {
        LOG_WARNING_F("Acceptable contact (dry skin?) (Z=%d)", impedanceValue);
        g_device_state.effector.type = EFFECTOR_CONTACT;
        g_device_state.effector.connected = true;
        g_device_state.effector.quality = 50;
        return CONTACT_ACCEPTABLE;
    } else if (impedanceValue > ELECTRODE_OPEN_CIRCUIT) {
        LOG_WARNING("No electrodes connected!");
        return CONTACT_OPEN;
    } else {
        LOG_WARNING_F("Poor electrode contact (Z=%d)", impedanceValue);
        return CONTACT_POOR;
    }
}

WrapStatus_t detect_wrap_applicator() {
    LOG_DEBUG("Detecting Wrap applicator...");
    
    // Pomiar rezystancji DC
    digitalWrite(PIN_WRAP_ENABLE, LOW);
    delay_us(100);
    
    int resistanceValue = analogRead(PIN_WRAP_CURRENT);
    
    // Krótki testowy impuls
    digitalWrite(PIN_WRAP_ENABLE, HIGH);
    analogWrite(PIN_PWM_OUTPUT, 64);
    delay_us(500);
    
    int impedanceValue = analogRead(PIN_WRAP_CURRENT);
    
    // Zatrzymaj
    analogWrite(PIN_PWM_OUTPUT, 0);
    digitalWrite(PIN_WRAP_ENABLE, LOW);
    
    // Pomiar temperatury
    float temperature = read_ntc_temperature(PIN_WRAP_TEMP);
    g_device_state.effector.temperature = temperature;
    
    // Decyzja
    if (temperature > WRAP_TEMP_MAX) {
        LOG_ERROR_F("Wrap overheating! T=%.1f°C", temperature);
        return WRAP_OVERTEMP;
    }
    
    if (resistanceValue < 20) {
        LOG_ERROR("Wrap coil SHORT CIRCUIT!");
        g_device_state.errorCount++;
        return WRAP_SHORT;
    } else if (resistanceValue >= WRAP_CONNECTED_MIN && resistanceValue <= WRAP_CONNECTED_MAX) {
        LOG_INFO_F("Wrap applicator ready (R=%d, T=%.1f°C)", resistanceValue, temperature);
        g_device_state.effector.type = EFFECTOR_WRAP;
        g_device_state.effector.connected = true;
        g_device_state.effector.impedance = resistanceValue;
        g_device_state.effector.quality = 90;
        return WRAP_READY;
    } else if (resistanceValue > WRAP_OPEN_CIRCUIT) {
        LOG_WARNING("Wrap coil OPEN CIRCUIT!");
        return WRAP_OPEN;
    } else {
        LOG_WARNING_F("Unknown wrap state (R=%d)", resistanceValue);
        g_device_state.effector.type = EFFECTOR_WRAP;
        g_device_state.effector.connected = true;
        g_device_state.effector.quality = 60;
        return WRAP_CONNECTED;
    }
}

EffectorType_t detect_effector() {
    LOG_DEBUG("Scanning for effectors...");
    g_device_state.detectionCount++;
    
    // Sprawdź każdy efektor sekwencyjnie
    if (detect_helmholtz_coil()) {
        g_current_effector = EFFECTOR_HELMHOLTZ;
        return EFFECTOR_HELMHOLTZ;
    }
    
    if (detect_otic_applicator()) {
        g_current_effector = EFFECTOR_OTIC;
        return EFFECTOR_OTIC;
    }
    
    ContactQuality_t contact = detect_electrodes();
    if (contact != CONTACT_OPEN && contact != CONTACT_SHORT) {
        g_current_effector = EFFECTOR_CONTACT;
        return EFFECTOR_CONTACT;
    }
    
    WrapStatus_t wrap = detect_wrap_applicator();
    if (wrap == WRAP_READY || wrap == WRAP_CONNECTED) {
        g_current_effector = EFFECTOR_WRAP;
        return EFFECTOR_WRAP;
    }
    
    // Sprawdź IR LED Strip
    if (detect_ir_led_strip()) {
        g_current_effector = EFFECTOR_IR_LED_STRIP;
        return EFFECTOR_IR_LED_STRIP;
    }
    
    // Brak podłączonego efektora
    g_current_effector = EFFECTOR_NONE;
    g_device_state.effector.connected = false;
    g_device_state.effector.type = EFFECTOR_NONE;
    
    LOG_INFO("No effector detected");
    return EFFECTOR_NONE;
}

// ============================================================================
// DETEKCJA IR LED STRIP
// ============================================================================

bool detect_ir_led_strip() {
    LOG_DEBUG("Detecting IR LED Strip...");
    
    // Sprawdź czy pin detekcji jest podłączony (pull-up)
    int detectValue = analogRead(PIN_IR_DETECT);
    
    // Jeśli pin IR_DETECT jest podłączony do masy przez rezystor, 
    // oznacza to podłączenie paska IR
    if (detectValue < 100) {
        // Spróbuj potwierdzić poprzez pomiar prądu jeśli dostępny
        #if defined(PIN_IR_CURRENT_SENSE)
        int currentSense = analogRead(PIN_IR_CURRENT_SENSE);
        
        if (currentSense > IR_CONNECTED_MIN && currentSense <= IR_CONNECTED_MAX) {
            LOG_INFO_F("IR LED Strip detected (ADC=%d)", currentSense);
            g_device_state.effector.type = EFFECTOR_IR_LED_STRIP;
            g_device_state.effector.connected = true;
            g_device_state.effector.impedance = currentSense;
            g_device_state.effector.quality = map(currentSense, IR_CONNECTED_MIN, IR_CONNECTED_MAX, 50, 100);
            return true;
        } else if (currentSense < IR_SHORT_CIRCUIT) {
            LOG_ERROR_F("IR LED Strip SHORT CIRCUIT! (ADC=%d)", currentSense);
            g_device_state.errorCount++;
            return false;
        } else if (currentSense > IR_OPEN_CIRCUIT) {
            LOG_WARNING("IR LED Strip not connected");
            return false;
        }
        #else
        // Brak pomiaru prądu - polegaj na detekcji cyfrowej
        LOG_INFO("IR LED Strip detected via digital pin");
        g_device_state.effector.type = EFFECTOR_IR_LED_STRIP;
        g_device_state.effector.connected = true;
        g_device_state.effector.quality = 80;
        return true;
        #endif
    }
    
    // Alternatywna metoda: test z włączonym PWM
    digitalWrite(IR_MODULATION_PIN, HIGH);
    delay_us(100);
    
    // Sprawdź reakcję na teście
    int testValue = analogRead(PIN_IR_DETECT);
    digitalWrite(IR_MODULATION_PIN, LOW);
    
    if (testValue >= IR_CONNECTED_MIN && testValue <= IR_CONNECTED_MAX) {
        LOG_INFO_F("IR LED Strip detected via PWM test (ADC=%d)", testValue);
        g_device_state.effector.type = EFFECTOR_IR_LED_STRIP;
        g_device_state.effector.connected = true;
        g_device_state.effector.impedance = testValue;
        g_device_state.effector.quality = 75;
        return true;
    }
    
    LOG_DEBUG("IR LED Strip not detected");
    return false;
}

// ============================================================================
// DETEKCJA BIOFEEDBACK
// ============================================================================

int read_gsr() {
    // Prosta implementacja - w rzeczywistości wymaga komunikacji I2C
    return analogRead(A0);  // Tymczasowe
}

float evaluate_gsr_quality() {
    uint32_t sum = 0;
    
    for (int i = 0; i < 100; i++) {
        sum += read_gsr();
        delay(10);
    }
    
    float mean = sum / 100.0f;
    
    if (mean < 100 || mean > 950) return 0.0f;
    
    return constrain((mean - 100) / 700.0f, 0.0f, 1.0f);
}

float evaluate_ppg_quality() {
    uint32_t startTime = millis();
    int pulseCount = 0;
    
    while (millis() - startTime < 3000) {
        if (digitalRead(PIN_PPG_INT) == LOW) {
            pulseCount++;
        }
        delay(1);
    }
    
    if (pulseCount < 3 || pulseCount > 200) return 0.0f;
    
    return constrain(pulseCount / 100.0f, 0.0f, 1.0f);
}

BioStatus_t detect_biofeedback_sensors(BioSensorStatus_t* status) {
    LOG_DEBUG("Detecting Biofeedback sensors...");
    
    // Test GSR (I2C)
    Wire.beginTransmission(GSR_I2C_ADDRESS);
    status->gsrConnected = (Wire.endTransmission() == 0);
    
    // Test PPG (SPI)
    digitalWrite(PPG_SPI_CS, LOW);
    delay_us(10);
    uint8_t ppgId = SPI.transfer(0x00);  // Read ID register
    digitalWrite(PPG_SPI_CS, HIGH);
    status->ppgConnected = (ppgId == 0x15);  // MAX30102 ID
    
    // Test Temp (I2C)
    Wire.beginTransmission(TEMP_I2C_ADDRESS);
    status->tempConnected = (Wire.endTransmission() == 0);
    
    // Quality check
    if (status->gsrConnected) {
        status->gsrQuality = evaluate_gsr_quality();
    }
    if (status->ppgConnected) {
        status->ppgQuality = evaluate_ppg_quality();
    }
    
    // Oblicz całkowite zaufanie
    int sensorsOk = (status->gsrConnected ? 1 : 0) +
                    (status->ppgConnected ? 1 : 0) +
                    (status->tempConnected ? 1 : 0);
    
    status->confidence = (sensorsOk / 3.0f) * 
                         ((status->gsrQuality + status->ppgQuality) / 2.0f);
    
    // Aktualizuj globalny stan
    g_device_state.biofeedback = *status;
    
    // Decyzja
    if (sensorsOk == 3 && status->confidence > HRV_CONFIDENCE_MIN) {
        LOG_INFO("Biofeedback: all sensors ready");
        g_device_state.biofeedbackEnabled = true;
        return BIO_READY;
    } else if (sensorsOk >= 2) {
        LOG_WARNING_F("Biofeedback: partial mode (%d/3)", sensorsOk);
        g_device_state.biofeedbackEnabled = true;
        return BIO_PARTIAL;
    } else if (!status->gsrConnected) {
        LOG_ERROR("Biofeedback: GSR sensor missing");
        return BIO_GSR_MISSING;
    } else if (!status->ppgConnected) {
        LOG_ERROR("Biofeedback: PPG sensor missing");
        return BIO_PPG_MISSING;
    } else {
        LOG_ERROR("Biofeedback: critical error");
        return BIO_ERROR;
    }
}

// ============================================================================
// ADAPTACJA TERAPII
// ============================================================================

void adapt_therapy_based_on_biofeedback(TherapyParams_t* params) {
    if (!g_device_state.biofeedbackEnabled) {
        return;
    }
    
    // Adaptacja na podstawie GSR
    if (g_bio_status.gsrConnected && g_bio_status.gsrValue > 0) {
        if (g_bio_status.gsrValue > 30.0f) {
            // Wysoki stres - zwiększ relaksację
            params->frequency = 10.0f;  // Hz (alfa)
            params->amplitude = 0.6f;
            LOG_INFO("High stress detected - relaxation adaptation");
        } else if (g_bio_status.gsrValue < 5.0f) {
            // Bardzo zrelaksowany - utrzymuj stan
            params->frequency = 7.83f;  // Schumann
            params->amplitude = 0.4f;
            LOG_INFO("Deep relaxation - maintenance mode");
        }
    }
    
    // Adaptacja na podstawie HRV
    if (g_bio_status.ppgConnected && g_bio_status.hrvValue > 0) {
        if (g_bio_status.hrvValue < 30) {
            // Niska HRV - stres
            params->frequency = 6.0f;  // Theta
            params->modType = MODULATION_AM;
            params->modFrequency = 0.5f;
            LOG_INFO("Low HRV - theta stimulation");
        } else if (g_bio_status.hrvValue > 70) {
            // Dobra HRV - relaks
            params->frequency = 10.0f;  // Alfa
            params->modType = MODULATION_NONE;
            LOG_INFO("Good HRV - maintenance alpha");
        }
    }
}

// ============================================================================
// GŁÓWNA PĘTLA
// ============================================================================

void device_detector_loop() {
    static uint32_t last_detection = 0;
    
    // Sprawdzaj urządzenia co 5 sekund
    if (millis() - last_detection >= 5000) {
        last_detection = millis();
        
        // Wykryj efektor jeśli nie jest aktywny
        if (g_device_state.effector.type == EFFECTOR_NONE) {
            detect_effector();
        }
        
        // Monitoruj biofeedback jeśli włączony
        if (g_device_state.biofeedbackEnabled) {
            detect_biofeedback_sensors(&g_bio_status);
            
            // Ciągła adaptacja terapii
            // TherapyParams_t* params = get_current_therapy_params();
            // adapt_therapy_based_on_biofeedback(params);
        }
    }
}

// ============================================================================
// FUNKCJE GETTERÓW I ERROR HANDLING
// ============================================================================

DeviceSystemState_t* get_device_system_state() {
    return &g_device_state;
}

// ============================================================================
// DETEKCJA GŁOŚNIKÓW PIEZO I WIBRATORÓW
// ============================================================================

bool detect_piezo_speaker() {
    LOG_DEBUG("Detecting Piezo Speaker...");
    
    // Sprawdź pin detekcji audio
    int detectValue = analogRead(PIN_AUDIO_DETECT);
    
    // Sprawdź czy wartość wskazuje na podłączenie
    if (detectValue >= PIEZO_CONNECTED_MIN && detectValue <= PIEZO_CONNECTED_MAX) {
        LOG_INFO_F("Piezo speaker detected (ADC=%d)", detectValue);
        return true;
    }
    
    // Sprawdź czy nie ma zwarcia
    if (detectValue < PIEZO_SHORT_CIRCUIT) {
        LOG_ERROR("Piezo speaker short circuit detected!");
        return false;
    }
    
    // Sprawdź czy nie ma przerwy
    if (detectValue > PIEZO_OPEN_CIRCUIT) {
        LOG_DEBUG("Piezo speaker not connected (open circuit)");
        return false;
    }
    
    return false;
}

bool detect_vibrator() {
    LOG_DEBUG("Detecting Vibrator...");
    
    // Sprawdź ciągłość obwodu wibratora przez pomiar impedancji
    pinMode(PIN_VIBRATOR_ENABLE, OUTPUT);
    digitalWrite(PIN_VIBRATOR_ENABLE, HIGH);
    delay(10);
    
    // Pomiar prądu/impedancji przez PIN_VIBRATOR_PWM (jako wejście analogowe)
    // Uwaga: wymaga dodatkowego układu pomiarowego
    int impedance = 500;  // Wartość domyślna - symulacja
    
    digitalWrite(PIN_VIBRATOR_ENABLE, LOW);
    
    if (impedance >= VIBRATOR_CONNECTED_MIN && impedance <= VIBRATOR_CONNECTED_MAX) {
        LOG_INFO_F("Vibrator detected (impedance=%d)", impedance);
        return true;
    }
    
    if (impedance > VIBRATOR_OPEN_CIRCUIT) {
        LOG_DEBUG("Vibrator not connected (open circuit)");
        return false;
    }
    
    return false;
}

bool piezo_set_tone(uint16_t freq_hz, uint8_t volume) {
    // Walidacja parametrów
    if (freq_hz < PIEZO_FREQ_MIN || freq_hz > PIEZO_FREQ_MAX) {
        LOG_ERROR_F("Invalid piezo frequency: %d Hz", freq_hz);
        return false;
    }
    
    if (volume > PIEZO_VOLUME_MAX) {
        volume = PIEZO_VOLUME_MAX;
    }
    
    // Włącz driver piezo
    pinMode(PIN_PIEZO_ENABLE, OUTPUT);
    digitalWrite(PIN_PIEZO_ENABLE, HIGH);
    
    // Ustaw częstotliwość na pinie PWM
    // Uwaga: Arduino Nano ma ograniczenia częstotliwości PWM
    // Dla dokładnej kontroli użyj Timer2
    #if defined(TCCR2A)
    TCCR2A = _BV(COM2A1) | _BV(WGM21) | _BV(WGM20);  // Fast PWM
    TCCR2B = _BV(CS20);  // No prescaler
    OCR2A = volume;  // Duty cycle
    // Częstotliwość: f = 31374 Hz dla prescaler=1
    // Dla niższych częstotliwości audio potrzebny jest inny timer lub DDS
    #endif
    
    LOG_INFO_F("Piezo tone set: %d Hz, volume=%d", freq_hz, volume);
    return true;
}

bool vibrator_set_intensity(uint8_t intensity) {
    if (intensity > VIBRATOR_INTENSITY_MAX) {
        intensity = VIBRATOR_INTENSITY_MAX;
    }
    
    // Włącz driver wibratora
    pinMode(PIN_VIBRATOR_ENABLE, OUTPUT);
    digitalWrite(PIN_VIBRATOR_ENABLE, HIGH);
    
    // Ustaw intensywność przez PWM
    analogWrite(PIN_VIBRATOR_PWM, intensity);
    
    LOG_INFO_F("Vibrator intensity set: %d", intensity);
    return true;
}

void piezo_stop() {
    digitalWrite(PIN_PIEZO_ENABLE, LOW);
    pinMode(PIN_PIEZO_PWM, INPUT);
    LOG_INFO("Piezo stopped");
}

void vibrator_stop() {
    digitalWrite(PIN_VIBRATOR_ENABLE, LOW);
    analogWrite(PIN_VIBRATOR_PWM, 0);
    LOG_INFO("Vibrator stopped");
}

/**
 * @brief Ustaw tryb verbose runtime
 */
void detection_set_verbose(bool enabled) {
    s_detection_config.verbose_mode = enabled;
    if (enabled) {
        LOG_INFO("Verbose mode enabled at runtime");
    } else {
        DBG_LOG("Verbose mode disabled");
    }
}

/**
 * @brief Sprawdź czy tryb verbose jest aktywny
 */
bool detection_is_verbose() {
    return s_detection_config.verbose_mode;
}

/**
 * @brief Pobierz licznik błędów detekcji
 */
uint32_t detection_get_error_count() {
    return s_error_count + g_device_state.errorCount;
}

/**
 * @brief Resetuj liczniki błędów
 */
void detection_reset_error_count() {
    s_error_count = 0;
    g_device_state.errorCount = 0;
    DBG_LOG("Error counters reset");
}

/**
 * @brief Pobierz opis błędu jako string z pamięci FLASH
 */
const char* detection_get_error_string(DetectionError_t error) {
    if (error >= 0 && error <= DETECT_ERR_CONFIG) {
        return (const char*)pgm_read_word(&s_error_strings[error]);
    }
    return "Unknown error";
}

// ============================================================================
// KONIEC PLIKU device_detector.cpp
// ============================================================================
