/**
 * @file device_detector.h
 * @brief System wykrywania i obsługi efektorów oraz sensorów
 * @version 1.0
 * 
 * Obsługuje automatyczne wykrywanie podłączonych urządzeń:
 * - Efektory: Helmholtz Coil, Otic, Contact Electrodes, Wrap
 * - Sensory: Biofeedback (GSR, HRV, Temp)
 */

#ifndef DEVICE_DETECTOR_H
#define DEVICE_DETECTOR_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "types.h"

// ============================================================================
// DEFINICJE PINÓW DLA EFEKTORÓW I SENSORÓW
// ============================================================================

// --- Cewka Helmholtza ---
#define PIN_HELMHOLTZ_DETECT    2     // Detekcja cyfrowa
#define PIN_HELMHOLTZ_SENSE     A1    // Pomiar prądu analogowy

// --- Aplikator Uszny (Otic) ---
#define PIN_OTIC_ENABLE         4     // Enable drivera
#define PIN_OTIC_IMPEDANCE      A2    // Pomiar impedancji

// --- Elektrody Kontaktowe ---
#define PIN_ELECTRODE_ENABLE    6     // Enable drivera
#define PIN_ELECTRODE_IMPEDANCE A3    // Pomiar impedancji

// --- Aplikator Okrężny (Wrap) ---
#define PIN_WRAP_ENABLE         8     // Enable drivera
#define PIN_WRAP_TEMP           A4    // Pomiar temperatury NTC
#define PIN_WRAP_CURRENT        A5    // Pomiar prądu

// --- Biofeedback Sensor ---
#define PIN_BIO_DETECT          7     // Detekcja cyfrowa
#define PIN_PPG_INT             5     // Interrupt PPG
#define PIN_GSR_DRDY            6     // Data ready GSR
#define PIN_NTC_TEMP            A0    // Temperatura skóry

// --- IR LED Strip ---
#define PIN_IR_DETECT           3     // Detekcja podłączenia IR LED strip
#define PIN_IR_CURRENT_SENSE    A6    // Pomiar prądu IR LED (jeśli dostępny)

// --- Głośniki Audio i Wibratory Piezo ---
#define PIN_PIEZO_ENABLE        7     // Enable drivera piezo/speaker
#define PIN_PIEZO_PWM           5     // PWM dla głośności
#define PIN_PIEZO_FREQ          6     // PWM dla częstotliwości audio
#define PIN_VIBRATOR_ENABLE     4     // Enable wibratora
#define PIN_VIBRATOR_PWM        8     // PWM dla intensywności wibracji
#define PIN_AUDIO_DETECT        A7    // Detekcja podłączenia głośnika

// Adresy I2C dla biofeedback
#define GSR_I2C_ADDRESS         0x48
#define TEMP_I2C_ADDRESS        0x4A
#define PPG_SPI_CS              10

// ============================================================================
// PROGI DETEKCJI
// ============================================================================

// Helmholtz Coil
#define HELMHOLTZ_CONNECTED_THRESHOLD  800
#define HELMHOLTZ_DISCONNECTED_VALUE   1023

// Otic Applicator
#define OTIC_CONNECTED_MIN    100
#define OTIC_CONNECTED_MAX    600
#define OTIC_OPEN_CIRCUIT     900

// Contact Electrodes
#define ELECTRODE_CONNECTED_MIN   200
#define ELECTRODE_CONNECTED_MAX   800
#define ELECTRODE_OPTIMAL_MIN     300
#define ELECTRODE_OPTIMAL_MAX     600
#define ELECTRODE_OPEN_CIRCUIT    950

// Wrap Applicator
#define WRAP_CONNECTED_MIN    50
#define WRAP_CONNECTED_MAX    400
#define WRAP_OPEN_CIRCUIT     900
#define WRAP_TEMP_MAX         45.0f

// IR LED Strip
#define IR_CONNECTED_MIN      100
#define IR_CONNECTED_MAX      800
#define IR_OPEN_CIRCUIT       950
#define IR_SHORT_CIRCUIT      50
#define IR_CURRENT_MAX        500   // mA - maksymalny prąd paska

// Piezo Speaker / Audio
#define PIEZO_CONNECTED_MIN   200
#define PIEZO_CONNECTED_MAX   800
#define PIEZO_OPEN_CIRCUIT    950
#define PIEZO_SHORT_CIRCUIT   50
#define PIEZO_FREQ_MIN        20    // Hz
#define PIEZO_FREQ_MAX        20000 // Hz
#define PIEZO_VOLUME_MAX      255   // PWM duty cycle

// Vibrator
#define VIBRATOR_CONNECTED_MIN 100
#define VIBRATOR_CONNECTED_MAX 600
#define VIBRATOR_OPEN_CIRCUIT  900
#define VIBRATOR_INTENSITY_MAX 255  // PWM duty cycle

// Biofeedback
#define GSR_SIGNAL_MIN        100
#define GSR_SIGNAL_MAX        900
#define PPG_SIGNAL_MIN        50000
#define HRV_CONFIDENCE_MIN    0.7f

// ============================================================================
// TYPY I STRUKTURY
// ============================================================================

// Typy efektorów
typedef enum {
    EFFECTOR_NONE = 0,
    EFFECTOR_HELMHOLTZ,
    EFFECTOR_OTIC,
    EFFECTOR_CONTACT,
    EFFECTOR_WRAP,
    EFFECTOR_IR_LED_STRIP,
    EFFECTOR_PIEZO_SPEAKER,
    EFFECTOR_VIBRATOR
} EffectorType_t;

// Stan połączenia elektrod
typedef enum {
    CONTACT_EXCELLENT = 0,
    CONTACT_GOOD,
    CONTACT_ACCEPTABLE,
    CONTACT_POOR,
    CONTACT_OPEN,
    CONTACT_SHORT
} ContactQuality_t;

// Stan aplikatora wrap
typedef enum {
    WRAP_READY = 0,
    WRAP_CONNECTED,
    WRAP_OPEN,
    WRAP_SHORT,
    WRAP_OVERTEMP
} WrapStatus_t;

// Stan biofeedback
typedef enum {
    BIO_READY = 0,
    BIO_PARTIAL,
    BIO_GSR_MISSING,
    BIO_PPG_MISSING,
    BIO_TEMP_MISSING,
    BIO_ERROR
} BioStatus_t;

// Status efektora
typedef struct {
    EffectorType_t type;
    bool connected;
    uint16_t impedance;      // Wartość ADC impedancji
    float temperature;       // Temperatura jeśli dostępna
    uint32_t last_check;     // Timestamp ostatniego sprawdzenia
    uint8_t quality;         // 0-100% jakości połączenia
} EffectorStatus_t;

// Status sensora biofeedback
typedef struct {
    bool gsrConnected;
    bool ppgConnected;
    bool tempConnected;
    float gsrValue;          // Przewodnictwo µS
    float hrvValue;          // HRV score 0-100
    float temperature;       // Temperatura °C
    float gsrQuality;        // 0.0 - 1.0
    float ppgQuality;        // 0.0 - 1.0
    float confidence;        // Overall confidence 0.0 - 1.0
} BioSensorStatus_t;

// Globalny stan systemu urządzeń
typedef struct {
    EffectorStatus_t effector;
    BioSensorStatus_t biofeedback;
    uint8_t activeEffectorMask;    // Bitmaska aktywnych efektorów
    bool biofeedbackEnabled;
    uint32_t detectionCount;
    uint32_t errorCount;
} DeviceSystemState_t;

// ============================================================================
// FUNKCJE PUBLICZNE
// ============================================================================

/**
 * @brief Inicjalizacja systemu detekcji urządzeń
 */
void device_detector_init();

/**
 * @brief Główna pętla systemu detekcji
 */
void device_detector_loop();

/**
 * @brief Wykryj podłączony efektor
 * @return EffectorType_t - typ wykrytego efektora
 */
EffectorType_t detect_effector();

/**
 * @brief Sprawdź status cewki Helmholtza
 * @return true jeśli podłączona
 */
bool detect_helmholtz_coil();

/**
 * @brief Sprawdź status aplikatora usznego
 * @return true jeśli podłączony
 */
bool detect_otic_applicator();

/**
 * @brief Sprawdź status i jakość elektrod kontaktowych
 * @return ContactQuality_t - jakość kontaktu
 */
ContactQuality_t detect_electrodes();

/**
 * @brief Sprawdź status aplikatora okrężnego
 * @return WrapStatus_t - status wrap
 */
WrapStatus_t detect_wrap_applicator();

/**
 * @brief Wykryj sensory biofeedback
 * @param status Wskaźnik do struktury statusu
 * @return BioStatus_t - stan systemu biofeedback
 */
BioStatus_t detect_biofeedback_sensors(BioSensorStatus_t* status);

/**
 * @brief Pobierz globalny stan systemu urządzeń
 * @return Wskaźnik do DeviceSystemState_t
 */
DeviceSystemState_t* get_device_system_state();

/**
 * @brief Oceń jakość sygnału GSR
 * @return float - jakość 0.0 - 1.0
 */
float evaluate_gsr_quality();

/**
 * @brief Oceń jakość sygnału PPG
 * @return float - jakość 0.0 - 1.0
 */
float evaluate_ppg_quality();

/**
 * @brief Odczytaj wartość GSR
 * @return int - wartość ADC
 */
int read_gsr();

/**
 * @brief Sprawdź status paska LED IR
 * @return true jeśli podłączony i sprawny
 */
bool detect_ir_led_strip();

/**
 * @brief Sprawdź podłączenie głośnika piezo
 * @return true jeśli podłączony
 */
bool detect_piezo_speaker();

/**
 * @brief Sprawdź podłączenie wibratora
 * @return true jeśli podłączony
 */
bool detect_vibrator();

/**
 * @brief Ustaw częstotliwość głośnika piezo
 * @param freq_hz Częstotliwość w Hz (20-20000)
 * @param volume Głośność 0-255
 * @return true jeśli sukces
 */
bool piezo_set_tone(uint16_t freq_hz, uint8_t volume);

/**
 * @brief Ustaw intensywność wibratora
 * @param intensity Intensywność 0-255
 * @return true jeśli sukces
 */
bool vibrator_set_intensity(uint8_t intensity);

/**
 * @brief Zatrzymaj dźwięk piezo
 */
void piezo_stop();

/**
 * @brief Zatrzymaj wibrator
 */
void vibrator_stop();

/**
 * @brief Adaptuj terapię na podstawie biofeedbacku
 * @param params Wskaźnik do parametrów terapii do modyfikacji
 */
void adapt_therapy_based_on_biofeedback(TherapyParams_t* params);

#endif // DEVICE_DETECTOR_H
