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
    EFFECTOR_WRAP
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
 * @brief Adaptuj terapię na podstawie biofeedbacku
 * @param params Wskaźnik do parametrów terapii do modyfikacji
 */
void adapt_therapy_based_on_biofeedback(TherapyParams_t* params);

#endif // DEVICE_DETECTOR_H
