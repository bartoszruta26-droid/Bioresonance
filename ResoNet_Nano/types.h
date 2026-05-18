/**
 * @file types.h
 * @brief Wspólne definicje typów, enumeracji i stałych dla projektu ResoNet Nano
 * @version 3.1 (Poprawiona spójność typów)
 */

#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>

// --- Konfiguracja Systemowa ---
#define FW_VERSION_MAJOR 3
#define FW_VERSION_MINOR 1
#define FW_VERSION_PATCH 0

// --- Definicje Pinów (Zgodne z hardware.md) ---
#define PIN_PWM_OUTPUT      9   // Timer1 OC1A
#define PIN_ENC28J60_CS     10  // Chip Select dla Ethernet
#define PIN_ENC28J60_MOSI   11  // SPI MOSI
#define PIN_ENC28J60_MISO   12  // SPI MISO
#define PIN_ENC28J60_SCK    13  // SPI Clock
#define PIN_ENC28J60_INT    2   // Interrupt dla Ethernet (nieużywany w polling)
#define PIN_TEMP_SENSOR     A0  // Czujnik temperatury
#define PIN_EMERGENCY_STOP  3   // Wejście fizyczne E-Stop (LOW = aktywne)
#define LED_BUILTIN_PIN     13

// --- Stałe Systemowe (Zgodne z README.md i hardware.md) ---
#define FREQ_MIN_HZ         0.1f
#define FREQ_MAX_HZ         500000.0f
#define FREQ_RESOLUTION_HZ  0.01f

#define TEMP_CRITICAL_C     85.0f
#define TEMP_WARNING_C      75.0f
#define TEMP_HYSTERESIS_C   5.0f

#define DUTY_CYCLE_MIN      1.0f
#define DUTY_CYCLE_MAX      99.0f

// --- Watchdog Timeouts (ms) ---
#define WDT_MAIN_TIMEOUT_MS     5000
#define WDT_NETWORK_TIMEOUT_MS  10000
#define WDT_THERAPY_TIMEOUT_MS  1000
#define WDT_COMMS_TIMEOUT_MS    30000

// --- EEPROM Addresses ---
#define EEPROM_RESET_COUNT_ADDR 0
#define EEPROM_LOCKOUT_FLAG_ADDR 1
#define EEPROM_MAGIC_BYTE       0xA5

// --- Enumeracje Stanów Systemu ---
typedef enum {
    STATE_BOOT = 0,
    STATE_IDLE,
    STATE_CONFIGURING,
    STATE_RUNNING,
    STATE_ERROR,
    STATE_SAFE_LOCKOUT,
    STATE_THERMAL_SHUTDOWN
} SystemState_t;

typedef enum {
    LOG_VERBOSE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
} LogLevel_t;

typedef enum {
    EVENT_NONE = 0,
    EVENT_CMD_RECEIVED,
    EVENT_CMD_EXECUTED,
    EVENT_CMD_FAILED,
    EVENT_NETWORK_CONNECT,
    EVENT_NETWORK_DISCONNECT,
    EVENT_NETWORK_CONNECTED,
    EVENT_NETWORK_DISCONNECTED,
    EVENT_PWM_START,
    EVENT_PWM_STOP,
    EVENT_SAFETY_TRIP,
    EVENT_WATCHDOG_FEED,
    EVENT_THERMAL_WARN,
    EVENT_MEMORY_LOW,
    EVENT_SYSTEM_RESET,
    EVENT_THERAPY_START,
    EVENT_THERAPY_STOP,
    EVENT_THERAPY_COMPLETE,
    EVENT_PACKET_RECEIVED
} EventType_t;

typedef enum {
    ERROR_NONE = 0,
    ERROR_INVALID_PARAM,
    ERROR_FREQ_OUT_OF_RANGE,
    ERROR_DUTY_CYCLE_INVALID,
    ERROR_CRC_MISMATCH,
    ERROR_BUFFER_OVERFLOW,
    ERROR_NETWORK_INIT_FAIL,
    ERROR_THERMAL_CRITICAL,
    ERROR_WATCHDOG_RESET,
    ERROR_EEPROM_FAIL
} ErrorCode_t;

typedef enum {
    MODULATION_NONE = 0,
    MODULATION_AM,
    MODULATION_FM,
    MODULATION_BURST
} ModulationType_t;

// --- Struktury Danych ---

typedef struct {
    uint32_t timestamp;
    LogLevel_t level;
    char message[64];
} LogEntry_t;

typedef struct {
    uint32_t timestamp;
    EventType_t type;
    int16_t data;
    uint8_t severity; // 0-255
} Event_t;

typedef struct {
    float frequency;          // Hz
    float dutyCycle;          // % (0.0 - 100.0)
    float amplitude;          // V (symulowane PWM)
    ModulationType_t modType;
    float modIndex;           // Indeks modulacji (0.0 - 1.0)
    float modFrequency;       // Częstotliwość modulacji (Hz)
} TherapyParams_t;

typedef struct {
    uint32_t uptime;
    SystemState_t state;
    float cpuTemp;
    uint16_t freeMemory;
    uint8_t watchdogResets;
    uint32_t eventsProcessed;
    uint32_t errorsTotal;
} SystemStatus_t;

// --- Makra Pomocnicze ---
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#endif // TYPES_H
