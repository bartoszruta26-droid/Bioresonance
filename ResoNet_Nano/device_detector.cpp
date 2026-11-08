/**
 * @file device_detector.cpp
 * @brief Implementacja systemu wykrywania efektorów i sensorów
 * @version 1.1 (Rozszerzona o debug, verbose, CLI, event handling)
 */

#include "device_detector.h"
#include <avr/pgmspace.h>
#include "logging_system.h"
#include "pwm_engine.h"
#include "event_system.h"
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
 * @brief Makro wewnętrzne dla debug logów
 */
#define DBG_LOG(msg) do { \
    if (s_detection_config.debug_enabled) { \
        LOG_DEBUG(msg); \
    } \
} while(0)

/**
 * @brief Obsługa zdarzeń detekcji
 */
void detection_handle_event(EventType_t event_type, EffectorType_t effector) {
    s_detection_events++;
    
    // Wyślij zdarzenie do systemu eventów
    event_push(event_type, EVENT_SEVERITY_INFO, (uint32_t)effector, "Detector event");
    
    // Loguj zdarzenie jeśli verbose
    if (s_detection_config.verbose_mode) {
        DBG_LOG("Event handled");
    }
}

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

/**
 * @brief Oblicz temperaturę z odczytu NTC
 */
// NTC 10k table derived from the existing Steinhart-Hart coefficients.
// Values are tenths of °C for ADC codes 32, 64, ... 992.
static const int16_t NTC_TEMP_C_X10[] PROGMEM = {
    1809, 1357, 1125, 970, 855, 762, 685, 618,
    559, 505, 456, 411, 368, 327, 288, 250,
    213, 177, 141, 105, 69, 32, -6, -45,
    -87, -131, -180, -236, -301, -385, -512
};

static int16_t read_ntc_temperature_c_x10(uint8_t pin) {
    const uint16_t raw = (uint16_t)analogRead(pin);
    if (raw <= 32U) return (int16_t)pgm_read_word(&NTC_TEMP_C_X10[0]);
    if (raw >= 992U) return (int16_t)pgm_read_word(&NTC_TEMP_C_X10[30]);

    const uint16_t offset = (uint16_t)(raw - 32U);
    const uint8_t index = (uint8_t)(offset / 32U);
    const uint8_t remainder = (uint8_t)(offset % 32U);
    const int16_t low = (int16_t)pgm_read_word(&NTC_TEMP_C_X10[index]);
    const int16_t high = (int16_t)pgm_read_word(&NTC_TEMP_C_X10[index + 1U]);
    const int32_t interpolated = (int32_t)low +
        ((int32_t)(high - low) * remainder) / 32L;
    return (int16_t)interpolated;
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
    DBG_LOG("Detection interval configured");
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
    
    DBG_LOG("Parsing CLI arguments");
    
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
                LOG_INFO("CLI detection interval configured");
            }
        }
        // Obsługa --calibrate / -c
        else if (strcmp(argv[i], "--calibrate") == 0 || strcmp(argv[i], "-c") == 0) {
            s_detection_config.force_calibration = true;
            LOG_INFO("CLI: Force calibration enabled");
        }
        else {
            LOG_WARNING("CLI unknown argument");
        }
    }
    
    return true;
}

// ============================================================================
// DETEKCJA EFEKTORÓW
// ============================================================================

// Active generators must never be disturbed by a diagnostic stimulus.
// In that state detector routines perform read-only sampling of their sense inputs.
static bool detector_passive_monitoring() {
    return pwm_is_running() || ir_led_is_running();
}

bool detect_helmholtz_coil() {
    LOG_DEBUG("Detecting Helmholtz coil...");
    const bool passive = detector_passive_monitoring();
    int senseValue;
    if (passive) {
        // Read-only monitoring: do not touch D9/Timer1 while therapy PWM or IR runs.
        senseValue = analogRead(PIN_HELMHOLTZ_SENSE);
    } else {
        analogWrite(PIN_PWM_OUTPUT, 128);
        delay_us(1000);
        senseValue = analogRead(PIN_HELMHOLTZ_SENSE);
        analogWrite(PIN_PWM_OUTPUT, 0);
    }
    
    if (senseValue < HELMHOLTZ_CONNECTED_THRESHOLD) {
        LOG_INFO("Helmholtz coil detected");
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
    const bool passive = detector_passive_monitoring();
    uint32_t sum = 0;
    if (!passive) {
        digitalWrite(PIN_OTIC_ENABLE, HIGH);
        analogWrite(PIN_PWM_OUTPUT, 64);
        delay_us(5000);
    }
    // While any generator is active, sample the sense input only.
    for (int i = 0; i < 10; i++) {
        sum += analogRead(PIN_OTIC_IMPEDANCE);
        if (!passive) delay_us(100);
    }
    int impedanceValue = sum / 10;
    if (!passive) {
        analogWrite(PIN_PWM_OUTPUT, 0);
        digitalWrite(PIN_OTIC_ENABLE, LOW);
    }
    
    if (impedanceValue >= OTIC_CONNECTED_MIN && impedanceValue <= OTIC_CONNECTED_MAX) {
        LOG_INFO("Otic applicator detected");
        g_device_state.effector.type = EFFECTOR_OTIC;
        g_device_state.effector.connected = true;
        g_device_state.effector.impedance = impedanceValue;
        g_device_state.effector.quality = 80;
        return true;
    } else if (impedanceValue > OTIC_OPEN_CIRCUIT) {
        LOG_WARNING("Otic applicator not connected");
        return false;
    } else {
        LOG_ERROR("Otic applicator short circuit");
        g_device_state.errorCount++;
        return false;
    }
}

ContactQuality_t detect_electrodes() {
    LOG_DEBUG("Detecting Contact electrodes...");
    const bool passive = detector_passive_monitoring();
    if (!passive) {
        digitalWrite(PIN_ELECTRODE_ENABLE, HIGH);
        analogWrite(PIN_PWM_OUTPUT, 32);
        delay_us(500);
        analogWrite(PIN_PWM_OUTPUT, 0);
    }
    // Passive samples never enable the electrode output or alter Timer1.
    uint32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogRead(PIN_ELECTRODE_IMPEDANCE);
        if (!passive) delay_us(50);
    }
    int impedanceValue = sum / 16;
    if (!passive) digitalWrite(PIN_ELECTRODE_ENABLE, LOW);
    
    g_device_state.effector.impedance = impedanceValue;
    
    // Ocena jakości
    if (impedanceValue < 100) {
        LOG_ERROR("Electrode short circuit");
        g_device_state.errorCount++;
        return CONTACT_SHORT;
    } else if (impedanceValue >= ELECTRODE_CONNECTED_MIN && impedanceValue <= ELECTRODE_OPTIMAL_MIN) {
        LOG_INFO("Good electrode contact");
        g_device_state.effector.type = EFFECTOR_CONTACT;
        g_device_state.effector.connected = true;
        g_device_state.effector.quality = 70;
        return CONTACT_GOOD;
    } else if (impedanceValue > ELECTRODE_OPTIMAL_MIN && impedanceValue <= ELECTRODE_OPTIMAL_MAX) {
        LOG_INFO("Excellent electrode contact");
        g_device_state.effector.type = EFFECTOR_CONTACT;
        g_device_state.effector.connected = true;
        g_device_state.effector.quality = 100;
        return CONTACT_EXCELLENT;
    } else if (impedanceValue > ELECTRODE_OPTIMAL_MAX && impedanceValue <= ELECTRODE_CONNECTED_MAX) {
        LOG_WARNING("Acceptable electrode contact");
        g_device_state.effector.type = EFFECTOR_CONTACT;
        g_device_state.effector.connected = true;
        g_device_state.effector.quality = 50;
        return CONTACT_ACCEPTABLE;
    } else if (impedanceValue > ELECTRODE_OPEN_CIRCUIT) {
        LOG_WARNING("No electrodes connected!");
        return CONTACT_OPEN;
    } else {
        LOG_WARNING("Poor electrode contact");
        return CONTACT_POOR;
    }
}

WrapStatus_t detect_wrap_applicator() {
    LOG_DEBUG("Detecting Wrap applicator...");
    const bool passive = detector_passive_monitoring();
    // Read the current-sense channel continuously; only apply a test stimulus
    // when no generator is active.
    if (!passive) {
        digitalWrite(PIN_WRAP_ENABLE, LOW);
        delay_us(100);
    }
    int resistanceValue = analogRead(PIN_WRAP_CURRENT);
    int impedanceValue = resistanceValue;
    if (!passive) {
        digitalWrite(PIN_WRAP_ENABLE, HIGH);
        analogWrite(PIN_PWM_OUTPUT, 64);
        delay_us(500);
        impedanceValue = analogRead(PIN_WRAP_CURRENT);
        analogWrite(PIN_PWM_OUTPUT, 0);
        digitalWrite(PIN_WRAP_ENABLE, LOW);
    }
    
    // Pomiar temperatury
    const int16_t temperature_c_x10 = read_ntc_temperature_c_x10(PIN_WRAP_TEMP);
    g_device_state.effector.temperature_c_x10 = temperature_c_x10;
    
    // Decyzja
    if (temperature_c_x10 > WRAP_TEMP_MAX_C_X10) {
        LOG_ERROR("Wrap overheating");
        return WRAP_OVERTEMP;
    }
    
    if (resistanceValue < 20) {
        LOG_ERROR("Wrap coil SHORT CIRCUIT!");
        g_device_state.errorCount++;
        return WRAP_SHORT;
    } else if (resistanceValue >= WRAP_CONNECTED_MIN && resistanceValue <= WRAP_CONNECTED_MAX) {
        LOG_INFO("Wrap applicator ready");
        g_device_state.effector.type = EFFECTOR_WRAP;
        g_device_state.effector.connected = true;
        g_device_state.effector.impedance = resistanceValue;
        g_device_state.effector.quality = 90;
        return WRAP_READY;
    } else if (resistanceValue > WRAP_OPEN_CIRCUIT) {
        LOG_WARNING("Wrap coil OPEN CIRCUIT!");
        return WRAP_OPEN;
    } else {
        LOG_WARNING("Unknown wrap state");
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
    const bool passive = detector_passive_monitoring();
    // Detection remains live during IR operation; this path is strictly read-only.
    int detectValue = analogRead(PIN_IR_DETECT);
    
    // Jeśli pin IR_DETECT jest podłączony do masy przez rezystor, 
    // oznacza to podłączenie paska IR
    if (detectValue < 100) {
        // Spróbuj potwierdzić poprzez pomiar prądu jeśli dostępny
        #if defined(PIN_IR_CURRENT_SENSE)
        int currentSense = analogRead(PIN_IR_CURRENT_SENSE);
        
        if (currentSense > IR_CONNECTED_MIN && currentSense <= IR_CONNECTED_MAX) {
            LOG_INFO("IR LED Strip detected");
            g_device_state.effector.type = EFFECTOR_IR_LED_STRIP;
            g_device_state.effector.connected = true;
            g_device_state.effector.impedance = currentSense;
            g_device_state.effector.quality = map(currentSense, IR_CONNECTED_MIN, IR_CONNECTED_MAX, 50, 100);
            return true;
        } else if (currentSense < IR_SHORT_CIRCUIT) {
            LOG_ERROR("IR LED Strip short circuit");
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
    
    // A stimulus on IR_MODULATION_PIN would collide with the active IR/electrode
    // control line. During operation, use only the passive detector sample.
    if (passive) {
        LOG_DEBUG("IR strip passive monitoring: no diagnostic stimulus applied");
        return false;
    }
    digitalWrite(IR_MODULATION_PIN, HIGH);
    delay_us(100);
    int testValue = analogRead(PIN_IR_DETECT);
    digitalWrite(IR_MODULATION_PIN, LOW);
    
    if (testValue >= IR_CONNECTED_MIN && testValue <= IR_CONNECTED_MAX) {
        LOG_INFO("IR LED Strip detected via PWM test");
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
    // No identified GSR front-end/ADC driver is implemented yet.
    // -1 explicitly means "measurement unavailable"; A0 is the skin NTC input.
    return -1;
}

uint8_t evaluate_ppg_quality_percent() {
    const uint32_t start_time = millis();
    uint16_t pulse_count = 0;

    while (millis() - start_time < 3000UL) {
        if (digitalRead(PIN_PPG_INT) == LOW) {
            ++pulse_count;
        }
        delay(1);
    }

    if (pulse_count < 3U || pulse_count > 200U) return 0;
    return pulse_count > 100U ? 100U : (uint8_t)pulse_count;
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
    status->gsrQuality = 0;
    status->gsrValue_x10 = 0; // No validated measurement; never infer conductance from A0.
    if (status->gsrConnected) {
        LOG_WARNING("GSR I2C device acknowledged; measurement unavailable until driver identification");
    }
    if (status->ppgConnected) {
        status->ppgQuality = evaluate_ppg_quality_percent();
    }
    
    // Oblicz całkowite zaufanie
    int sensorsOk = (status->gsrConnected ? 1 : 0) +
                    (status->ppgConnected ? 1 : 0) +
                    (status->tempConnected ? 1 : 0);
    
    // Integer percentage: sensor coverage × mean of the two quality scores.
    status->confidence = (uint8_t)(
        (sensorsOk * ((uint16_t)status->gsrQuality + status->ppgQuality)) / 6U);
    
    // Aktualizuj globalny stan
    g_device_state.biofeedback = *status;
    
    // Decyzja
    if (sensorsOk == 3 && status->confidence > HRV_CONFIDENCE_MIN) {
        LOG_INFO("Biofeedback: all sensors ready");
        g_device_state.biofeedbackEnabled = true;
        return BIO_READY;
    } else if (sensorsOk >= 2) {
        LOG_WARNING("Biofeedback partial mode");
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
    // Disabled until validated physiological measurements and a reviewed
    // stimulation policy are available. This function intentionally changes nothing.
    (void)params;
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
        LOG_INFO("Piezo speaker detected");
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
        LOG_INFO("Vibrator detected");
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
        LOG_ERROR("Invalid piezo frequency");
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
    
    LOG_INFO("Piezo tone configured");
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
    
    LOG_INFO("Vibrator intensity configured");
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
