/**
 * @file device_detector.cpp
 * @brief Implementacja systemu wykrywania efektorów i sensorów
 * @version 1.1 (Rozszerzona o debug, verbose, CLI, event handling)
 */

#include "device_detector.h"
#include <avr/pgmspace.h>
#include "logging_system.h"
#include "event_system.h"
#include "effector_presence.h"
#include <string.h>   // Dla strcmp, strncmp, memcpy, strncpy
#include <avr/pgmspace.h>  // Dla PROGMEM, pgm_read_word

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
    // D4, D6 and D8 are shared with other subsystems in the current map.
    // The detector must not claim or drive these pins during initialization.
    pinMode(PIN_BIO_DETECT, INPUT_PULLUP);
    pinMode(PIN_PPG_INT, INPUT);
    pinMode(PPG_SPI_CS, OUTPUT);
    // D3 is also IR_LED_PIN / Timer2 OC2B. Never reconfigure it as a detector input.
    
    // Do not force shared effector-enable lines LOW: D4/D6/D8 are
    // multiplexed with vibrator, IR modulation, GSR and debug functions.
    // Their safe idle states require a resolved hardware pin map.
    digitalWrite(PPG_SPI_CS, HIGH);
    // Do not write PIN_IR_DETECT: it aliases the live IR carrier output on D3.
    
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

bool detect_helmholtz_coil() {
    LOG_DEBUG("Detecting Helmholtz coil...");
    // Passive-only sampling: D9/Timer1 belongs exclusively to the main PWM
    // generator. Detector code must never borrow or reconfigure that resource.
    const int senseValue = analogRead(PIN_HELMHOLTZ_SENSE);
    
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
    // Passive-only sampling. The current firmware has no independent,
    // validated OTIC diagnostic excitation channel.
    uint32_t sum = 0;
    for (uint8_t i = 0; i < 10; ++i) {
        sum += analogRead(PIN_OTIC_IMPEDANCE);
    }
    const int impedanceValue = static_cast<int>(sum / 10U);
    
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
    LOG_DEBUG("Monitoring Contact electrodes...");
    // D6 is shared with IR_MODULATION_PIN. Never toggle the electrode enable
    // line here: it would corrupt a running IR waveform. Use read-only samples.
    uint32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogRead(PIN_ELECTRODE_IMPEDANCE);
    }
    const int impedanceValue = static_cast<int>(sum / 16U);
    g_device_state.effector.impedance = static_cast<uint16_t>(impedanceValue);

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
    // Passive-only monitoring. Do not borrow D9/Timer1 or toggle the
    // Wrap enable line from the detector; that line is shared in current map.
    const int resistanceValue = analogRead(PIN_WRAP_CURRENT);
    
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
    LOG_DEBUG("Monitoring IR LED Strip...");
    // D3 is the Timer2 OC2B carrier output, not an independent detector input.
    // D6 is shared with electrode enable / IR modulation. Never reconfigure D3
    // or inject a test pulse on D6 from the detector.
#if defined(PIN_IR_CURRENT_SENSE)
    const int currentSense = analogRead(PIN_IR_CURRENT_SENSE);
    if (currentSense >= IR_CONNECTED_MIN && currentSense <= IR_CONNECTED_MAX) {
        LOG_INFO("IR LED Strip detected via independent current-sense input");
        g_device_state.effector.type = EFFECTOR_IR_LED_STRIP;
        g_device_state.effector.connected = true;
        g_device_state.effector.impedance = static_cast<uint16_t>(currentSense);
        g_device_state.effector.quality = static_cast<uint8_t>(
            map(currentSense, IR_CONNECTED_MIN, IR_CONNECTED_MAX, 50, 100));
        return true;
    }
    if (currentSense < IR_SHORT_CIRCUIT) {
        LOG_ERROR("IR LED Strip current-sense indicates short circuit");
        g_device_state.errorCount++;
    } else if (currentSense > IR_OPEN_CIRCUIT) {
        LOG_DEBUG("IR LED Strip current-sense indicates open circuit");
    }
#else
    LOG_WARNING("IR strip detection unavailable: no independent current-sense input");
#endif
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
    // A level sampled in a blocking 3-second polling loop is not a pulse count
    // and stalls PWM, IR servicing, Ethernet and sample acquisition. Until a
    // real PPG driver supplies timestamped samples, quality is unavailable.
    return 0;
}

BioStatus_t detect_biofeedback_sensors(BioSensorStatus_t* status) {
    LOG_DEBUG("Detecting Biofeedback sensors...");
    
    // Test GSR (I2C)
    Wire.beginTransmission(GSR_I2C_ADDRESS);
    status->gsrConnected = (Wire.endTransmission() == 0);
    
    // PPG probing is intentionally disabled: PPG_SPI_CS is D10, which is
    // also PIN_ENC28J60_CS. Pulling it LOW would select both SPI devices and
    // corrupt Ethernet/PPG transactions. A dedicated PPG CS pin is required.
    status->ppgConnected = false;
    status->ppgQuality = 0;
    LOG_WARNING("PPG unavailable: SPI chip-select is shared with ENC28J60");
    
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
    // Rozłóż diagnostykę pięciu torów na osobne wywołania pętli.
    // Każdy przebieg wykonuje najwyżej jedną grupę odczytów ADC; nie ma delay().
    static uint32_t last_effector_step = 0;
    static uint32_t last_bio_scan = 0;
    static uint8_t scan_phase = 0;
    static const uint8_t scan_phase_count = 5;
    const uint32_t now = millis();
    const uint32_t configured_seconds =
        (s_detection_config.detection_interval == 0U) ? 5U :
        (uint32_t)s_detection_config.detection_interval;
    const uint32_t phase_interval_ms =
        ((configured_seconds + scan_phase_count - 1U) / scan_phase_count) * 1000UL;

    if ((uint32_t)(now - last_effector_step) >= phase_interval_ms) {
        last_effector_step = now;
        EffectorType_t checked_type = EFFECTOR_NONE;
        bool present = false;

        switch (scan_phase) {
            case 0:
                checked_type = EFFECTOR_HELMHOLTZ;
                present = detect_helmholtz_coil();
                break;
            case 1:
                checked_type = EFFECTOR_OTIC;
                present = detect_otic_applicator();
                break;
            case 2: {
                checked_type = EFFECTOR_CONTACT;
                const ContactQuality_t contact = detect_electrodes();
                present = (contact != CONTACT_OPEN && contact != CONTACT_SHORT);
                break;
            }
            case 3: {
                checked_type = EFFECTOR_WRAP;
                const WrapStatus_t wrap = detect_wrap_applicator();
                present = (wrap == WRAP_READY || wrap == WRAP_CONNECTED);
                break;
            }
            default:
                checked_type = EFFECTOR_IR_LED_STRIP;
                present = detect_ir_led_strip();
                break;
        }

        g_device_state.activeEffectorMask =
            effector_presence::set(g_device_state.activeEffectorMask,
                                   static_cast<uint8_t>(checked_type), present);

        scan_phase++;
        if (scan_phase >= scan_phase_count) {
            scan_phase = 0;
            g_device_state.detectionCount++;
        }

        // activeEffectorMask zachowuje równoczesną obecność wielu urządzeń.
        // EffectorStatus_t jest historycznie pojedynczy, więc type wskazuje
        // najniższy aktywny identyfikator, a szczegółowy pomiar ostatni skan.
        const uint8_t primary = effector_presence::primary(
            g_device_state.activeEffectorMask, static_cast<uint8_t>(EFFECTOR_VIBRATOR));
        g_device_state.effector.type = static_cast<EffectorType_t>(primary);
        g_device_state.effector.connected = (primary != EFFECTOR_NONE);
        g_device_state.effector.last_check = now;
    }

    // Skan czujników jest osobnym zadaniem czasowym, a nie częścią skanu efektorów.
    // Funkcja wykonuje krótkie transakcje I2C/SPI; nie czeka na próbki PPG.
    if ((uint32_t)(now - last_bio_scan) >= 5000UL) {
        last_bio_scan = now;
        detect_biofeedback_sensors(&g_bio_status);
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
    LOG_DEBUG("Monitoring Vibrator...");
    // No independent current/continuity feedback input is defined. Do not
    // assert that a vibrator is connected from a fabricated constant.
    return false;
}

bool piezo_set_tone(uint16_t freq_hz, uint8_t volume) {
    if (freq_hz < PIEZO_FREQ_MIN || freq_hz > PIEZO_FREQ_MAX) {
        LOG_ERROR("Invalid piezo frequency");
        return false;
    }
    (void)volume;
    // Timer2 is owned by the IR carrier generator (OC2B / D3, OCR2A TOP).
    // The previous piezo implementation rewrote TCCR2A/B and OCR2A, silently
    // destroying IR timing. A dedicated timer or external audio driver is
    // required before this API can safely enable piezo output.
    LOG_WARNING("Piezo unavailable: Timer2 is reserved for IR carrier");
    return false;
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
