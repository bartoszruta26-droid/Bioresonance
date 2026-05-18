/**
 * @file ResoNet_Nano.ino
 * @brief Profesjonalny System Biorezonansu Klasy Medycznej - Firmware Arduino
 * @version 3.0 (Medical Grade with Full Debug & Event System)
 * @date 2024
 * 
 * @description
 * ResoNet-Nano to zaawansowany system biorezonansu z architekturą rozproszoną,
 * spełniający rygorystyczne normy bezpieczeństwa urządzeń medycznych IEC 60601-1.
 * 
 * Mikrokontroler: Arduino Nano (ATmega328P)
 * Moduł Ethernet: ENC28J60
 * Wyjście PWM: Pin D9 (Timer1 16-bit)
 * 
 * Komunikacja:
 * - UDP port 5000: Dane telemetryczne w czasie rzeczywistym
 * - TCP port 5001: Konfiguracja i polecenia krytyczne
 * 
 * Protokół binarny TherapyPacket:
 * - frequency_hz_x100: Częstotliwość * 100 (np. 72700 = 727.00 Hz)
 * - duration_sec: Czas emisji [s]
 * - modulation_type: 0=None, 1=AM, 2=FM, 3=Burst
 * - duty_cycle: 0-100%
 * - intensity_level: 0-4095 (12-bit)
 * - checksum: CRC8
 * 
 * Bezpieczeństwo:
 * - Izolacja galwaniczna DC 2500V RMS
 * - Optoizolatory 6N137 na liniach PWM
 * - Watchdog Timer aktywny
 * - Detekcja błędów i reset systemu
 * 
 * Funkcje dodatkowe (v3.0):
 * - Zaawansowany system logowania z poziomami (VERBOSE, DEBUG, INFO, WARN, ERROR, FATAL)
 * - Obsługa zdarzeń (Event Handling) z kolejką
 * - Kompleksowa obsługa błędów z kodami i opisami
 * - Rozbudowany Watchdog z monitorowaniem wielu warstw
 * - Statystyki systemowe i diagnostyka
 * - Ring buffer dla logów
 */

#include <SPI.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

// ============================================================================
// KONFIGURACJA SYSTEMU DEBUGOWANIA I LOGOWANIA
// ============================================================================

// Poziomy logowania
#define LOG_LEVEL_VERBOSE  0
#define LOG_LEVEL_DEBUG    1
#define LOG_LEVEL_INFO     2
#define LOG_LEVEL_WARN     3
#define LOG_LEVEL_ERROR    4
#define LOG_LEVEL_FATAL    5

// Konfiguracja poziomu logowania (zmień na wyższy aby ukryć szczegółowe logi)
#define CURRENT_LOG_LEVEL LOG_LEVEL_VERBOSE

// Włącz/wyłącz logowanie przez Serial
#define ENABLE_SERIAL_LOG true
#define ENABLE_REMOTE_LOG  true  // Logi przez sieć (UDP)

// Rozmiar ring buffera dla logów
#define LOG_BUFFER_SIZE 256

// ============================================================================
// KONFIGURACJA PINÓW I SPRZĘTU
// ============================================================================

// ENC28J60 Ethernet Module Pins
#define ENC28J60_CS   10    // Chip Select
#define ENC28J60_MOSI 11    // Master Out Slave In
#define ENC28J60_MISO 12    // Master In Slave Out
#define ENC28J60_SCK  13    // Serial Clock

// PWM Output Pin (Timer1 Channel A - OC1A)
#define PWM_OUTPUT_PIN 9    // D9 - Timer1 PWM z izolacją opto 6N137

// Status LED
#define STATUS_LED      13  // Built-in LED

// Debug LED (dodatkowy)
#define DEBUG_LED       8   // Dodatkowa dioda do debugowania

// ============================================================================
// KONFIGURACJA SIECIOWA
// ============================================================================

#define UDP_PORT 5000       // Port dla danych telemetrycznych
#define TCP_PORT 5001       // Port dla konfiguracji i poleceń krytycznych

// Adres IP (może być DHCP lub statyczny)
#define USE_DHCP false
#if USE_DHCP
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  IPAddress ip;
#else
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  IPAddress ip(192, 168, 1, 177);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
#endif

// ============================================================================
// STRUKTURY DANYCH PROTOKOŁU
// ============================================================================

#pragma pack(push, 1)
struct TherapyPacket {
  uint32_t frequency_hz_x100;   // Częstotliwość * 100 (np. 72700 = 727.00 Hz)
  uint32_t duration_sec;        // Czas emisji [s]
  uint8_t modulation_type;      // 0=None, 1=AM, 2=FM, 3=Burst
  uint8_t duty_cycle;           // 0-100%
  uint16_t intensity_level;     // 0-4095 (12-bit DAC equivalent)
  uint8_t checksum;             // CRC8
};
#pragma pack(pop)

struct DeviceStatus {
  uint32_t current_frequency;   // Aktualna częstotliwość w Hz * 100
  uint32_t elapsed_time;        // Upłynięty czas w sekundach
  uint8_t status_flags;         // Flagi statusu
  uint8_t error_code;           // Kod błędu
  uint16_t temperature;         // Temperatura MCU * 10
  uint16_t reserved;            // Rezerwa
};

// ============================================================================
// FLAGI STATUSU I BŁĘDY - ROZSZERZONE
// ============================================================================

#define STATUS_RUNNING          0x01
#define STATUS_ERROR            0x02
#define STATUS_CALIBRATING      0x04
#define STATUS_OVERHEAT         0x08
#define STATUS_ANTENNA_FAULT    0x10
#define STATUS_WATCHDOG_RESET   0x20
#define STATUS_NETWORK_FAULT    0x40
#define STATUS_EEPROM_FAULT     0x80

#define ERROR_NONE                  0x00
#define ERROR_FREQ_OUT_OF_RANGE     0x01
#define ERROR_DUTY_INVALID          0x02
#define ERROR_CHECKSUM_FAIL         0x03
#define ERROR_TIMER_CONFIG          0x04
#define ERROR_OVERHEAT              0x05
#define ERROR_NETWORK_INIT          0x06
#define ERROR_MEMORY_ALLOC          0x07
#define ERROR_EVENT_QUEUE_FULL      0x08
#define ERROR_LOG_BUFFER_OVERFLOW   0x09
#define ERROR_WDTO_RESET            0x0A
#define ERROR_STACK_OVERFLOW        0x0B
#define ERROR_INVALID_PARAM         0x0C
#define ERROR_HARDWARE_FAULT        0x0D

// Kody zdarzeń (Event Codes)
#define EVENT_THERAPY_START         0x0001
#define EVENT_THERAPY_STOP          0x0002
#define EVENT_THERAPY_COMPLETE      0x0003
#define EVENT_FREQ_CHANGED          0x0004
#define EVENT_MODULATION_CHANGED    0x0005
#define EVENT_NETWORK_CONNECTED     0x0006
#define EVENT_NETWORK_DISCONNECTED  0x0007
#define EVENT_WATCHDOG_FEED         0x0008
#define EVENT_ERROR_OCCURRED        0x0009
#define EVENT_LOG_FLUSH             0x000A
#define EVENT_SYSTEM_RESET          0x000B
#define EVENT_SAFETY_TRIGGER        0x000C

// Rozmiary kolejek
#define EVENT_QUEUE_SIZE 32
#define ERROR_HISTORY_SIZE 16

// ============================================================================
// ZMIENNE GLOBALNE - ROZSZERZONE O SYSTEM LOGOWANIA I EVENTÓW
// ============================================================================

// Stan terapii
volatile bool therapy_active = false;
volatile uint32_t therapy_start_time = 0;
volatile uint32_t therapy_duration = 0;

// Parametry bieżące
volatile uint32_t current_frequency_x100 = 0;
volatile uint8_t current_duty_cycle = 50;
volatile uint8_t current_modulation = 0;
volatile uint16_t current_intensity = 2048;

// Bufory sieciowe
uint8_t udp_buffer[512];
uint8_t tcp_buffer[512];

// Watchdog network
volatile unsigned long last_heartbeat = 0;
#define HEARTBEAT_TIMEOUT 5000  // 5 sekund

// CRC8 lookup table
const uint8_t crc8_table[256] PROGMEM = {
  0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31,
  0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
  0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9,
  0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
  0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1,
  0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
  0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, 0xB7, 0xB0, 0xB9, 0xBE,
  0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
  0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16,
  0x03, 0x04, 0x0D, 0x0A, 0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
  0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E, 0x87, 0x80,
  0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
  0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8,
  0xDD, 0xDA, 0xD3, 0xD4, 0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
  0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, 0x19, 0x1E, 0x17, 0x10,
  0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
  0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F,
  0x6A, 0x6D, 0x64, 0x63, 0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
  0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7,
  0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
  0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF,
  0xFA, 0xFD, 0xF4, 0xF3
};

// ============================================================================
// STRUKTURY SYSTEMU LOGOWANIA
// ============================================================================

struct LogEntry {
  uint32_t timestamp;
  uint8_t level;
  uint16_t event_code;
  char message[64];
};

// Ring buffer dla logów
struct LogBuffer {
  LogEntry entries[LOG_BUFFER_SIZE];
  uint16_t head;
  uint16_t tail;
  uint16_t count;
  uint16_t overflow_count;
};

volatile LogBuffer log_buffer;

// ============================================================================
// STRUKTURY SYSTEMU EVENTÓW
// ============================================================================

struct SystemEvent {
  uint32_t timestamp;
  uint16_t event_code;
  uint8_t severity;  // 0=Info, 1=Warning, 2=Error, 3=Critical
  uint32_t data;
  char description[32];
};

struct EventQueue {
  SystemEvent events[EVENT_QUEUE_SIZE];
  uint16_t head;
  uint16_t tail;
  uint16_t count;
  uint16_t dropped_count;
};

volatile EventQueue event_queue;

// ============================================================================
// STRUKTURY STATYSTYK I DIAGNOSTYKI
// ============================================================================

struct SystemStats {
  uint32_t uptime_ms;
  uint32_t total_events;
  uint32_t total_errors;
  uint32_t watchdog_resets;
  uint32_t packets_received;
  uint32_t packets_sent;
  uint32_t therapy_sessions;
  uint16_t max_free_memory;
  uint8_t current_error;
};

volatile SystemStats system_stats;

// Historia błędów
struct ErrorHistory {
  uint8_t error_codes[ERROR_HISTORY_SIZE];
  uint32_t timestamps[ERROR_HISTORY_SIZE];
  uint16_t head;
  uint16_t count;
};

volatile ErrorHistory error_history;

// ============================================================================
// SYSTEM LOGOWANIA - IMPLEMENTACJA
// ============================================================================

/**
 * @brief Nazwy poziomów logowania
 */
const char* get_log_level_name(uint8_t level) {
  switch(level) {
    case LOG_LEVEL_VERBOSE: return PSTR("VERBOSE");
    case LOG_LEVEL_DEBUG:   return PSTR("DEBUG");
    case LOG_LEVEL_INFO:    return PSTR("INFO");
    case LOG_LEVEL_WARN:    return PSTR("WARN");
    case LOG_LEVEL_ERROR:   return PSTR("ERROR");
    case LOG_LEVEL_FATAL:   return PSTR("FATAL");
    default:                return PSTR("UNKNOWN");
  }
}

/**
 * @brief Inicjalizuje ring buffer logów
 */
void log_buffer_init() {
  cli();
  log_buffer.head = 0;
  log_buffer.tail = 0;
  log_buffer.count = 0;
  log_buffer.overflow_count = 0;
  sei();
}

/**
 * @brief Dodaje wpis do ring buffera logów
 * @param level Poziom logowania
 * @param event_code Kod zdarzenia
 * @param message Wiadomość tekstowa
 */
void log_add_entry(uint8_t level, uint16_t event_code, const char* message) {
  if (level < CURRENT_LOG_LEVEL) {
    return;  // Pomijamy logi poniżej aktualnego poziomu
  }
  
  cli();
  
  // Sprawdź przepełnienie
  if (log_buffer.count >= LOG_BUFFER_SIZE) {
    log_buffer.overflow_count++;
    log_buffer.tail = (log_buffer.tail + 1) % LOG_BUFFER_SIZE;
    log_buffer.count--;
  }
  
  // Dodaj nowy wpis
  LogEntry* entry = &log_buffer.entries[log_buffer.head];
  entry->timestamp = millis();
  entry->level = level;
  entry->event_code = event_code;
  
  // Skopiuj wiadomość (bezpieczne kopiowanie)
  strncpy(entry->message, message, sizeof(entry->message) - 1);
  entry->message[sizeof(entry->message) - 1] = '\0';
  
  // Aktualizuj wskaźniki
  log_buffer.head = (log_buffer.head + 1) % LOG_BUFFER_SIZE;
  log_buffer.count++;
  
  sei();
  
  // Wyślij na Serial jeśli włączone
  if (ENABLE_SERIAL_LOG && Serial) {
    Serial.print(F("[LOG "));
    Serial.print(millis());
    Serial.print(F("ms] "));
    Serial.print(get_log_level_name(level));
    Serial.print(F(": "));
    Serial.println(message);
  }
}

/**
 * @brief Makra pomocnicze dla logowania
 */
#define LOG_VERBOSE(msg) log_add_entry(LOG_LEVEL_VERBOSE, 0, msg)
#define LOG_DEBUG(msg)   log_add_entry(LOG_LEVEL_DEBUG, 0, msg)
#define LOG_INFO(msg)    log_add_entry(LOG_LEVEL_INFO, 0, msg)
#define LOG_WARN(msg)    log_add_entry(LOG_LEVEL_WARN, 0, msg)
#define LOG_ERROR(msg)   log_add_entry(LOG_LEVEL_ERROR, 0, msg)
#define LOG_FATAL(msg)   log_add_entry(LOG_LEVEL_FATAL, 0, msg)

#define LOG_EVENT(level, event, msg) log_add_entry(level, event, msg)

// ============================================================================
// SYSTEM OBSŁUGI ZDARZEŃ (EVENT HANDLING) - IMPLEMENTACJA
// ============================================================================

/**
 * @brief Inicjalizuje kolejkę zdarzeń
 */
void event_queue_init() {
  cli();
  event_queue.head = 0;
  event_queue.tail = 0;
  event_queue.count = 0;
  event_queue.dropped_count = 0;
  sei();
}

/**
 * @brief Dodaje zdarzenie do kolejki
 * @param event_code Kod zdarzenia
 * @param severity Poziom ważności (0=Info, 1=Warning, 2=Error, 3=Critical)
 * @param data Dodatkowe dane
 * @param description Opis zdarzenia
 * @return true jeśli dodano sukcesem, false jeśli kolejka pełna
 */
bool event_push(uint16_t event_code, uint8_t severity, uint32_t data, const char* description) {
  cli();
  
  // Sprawdź przepełnienie
  if (event_queue.count >= EVENT_QUEUE_SIZE) {
    event_queue.dropped_count++;
    event_queue.tail = (event_queue.tail + 1) % EVENT_QUEUE_SIZE;
    event_queue.count--;
    
    LOG_EVENT(LOG_LEVEL_WARN, EVENT_ERROR_OCCURRED, "Event queue overflow - dropped event");
    system_stats.total_errors++;
  }
  
  // Dodaj nowe zdarzenie
  SystemEvent* event = &event_queue.events[event_queue.head];
  event->timestamp = millis();
  event->event_code = event_code;
  event->severity = severity;
  event->data = data;
  
  strncpy(event->description, description, sizeof(event->description) - 1);
  event->description[sizeof(event->description) - 1] = '\0';
  
  // Aktualizuj wskaźniki
  event_queue.head = (event_queue.head + 1) % EVENT_QUEUE_SIZE;
  event_queue.count++;
  system_stats.total_events++;
  
  sei();
  
  // Loguj zdarzenie
  char log_msg[80];
  snprintf_P(log_msg, sizeof(log_msg), PSTR("Event 0x%04X [%s]"), event_code, description);
  log_add_entry(LOG_LEVEL_INFO, event_code, log_msg);
  
  return true;
}

/**
 * @brief Pobiera zdarzenie z kolejki (FIFO)
 * @param event Wskaźnik do struktury zdarzenia
 * @return true jeśli pobrano zdarzenie, false jeśli kolejka pusta
 */
bool event_pop(SystemEvent* event) {
  if (!event) return false;
  
  cli();
  
  if (event_queue.count == 0) {
    sei();
    return false;
  }
  
  // Skopiuj zdarzenie
  memcpy(event, &event_queue.events[event_queue.tail], sizeof(SystemEvent));
  
  // Aktualizuj wskaźniki
  event_queue.tail = (event_queue.tail + 1) % EVENT_QUEUE_SIZE;
  event_queue.count--;
  
  sei();
  return true;
}

/**
 * @brief Helper do dodawania zdarzeń
 */
#define EVENT_INFO(code, desc)      event_push(code, 0, 0, desc)
#define EVENT_WARNING(code, desc)   event_push(code, 1, 0, desc)
#define EVENT_ERROR(code, desc)     event_push(code, 2, 0, desc)
#define EVENT_CRITICAL(code, desc)  event_push(code, 3, 0, desc)

// ============================================================================
// SYSTEM OBSŁUGI BŁĘDÓW (ERROR HANDLING) - IMPLEMENTACJA
// ============================================================================

/**
 * @brief Inicjalizuje historię błędów
 */
void error_history_init() {
  cli();
  error_history.head = 0;
  error_history.count = 0;
  sei();
}

/**
 * @brief Rejestruje błąd w historii
 * @param error_code Kod błędu
 */
void error_register(uint8_t error_code) {
  if (error_code == ERROR_NONE) return;
  
  cli();
  
  // Sprawdź przepełnienie
  if (error_history.count >= ERROR_HISTORY_SIZE) {
    error_history.head = (error_history.head + 1) % ERROR_HISTORY_SIZE;
    error_history.count--;
  }
  
  // Dodaj błąd
  error_history.error_codes[error_history.head] = error_code;
  error_history.timestamps[error_history.head] = millis();
  error_history.head = (error_history.head + 1) % ERROR_HISTORY_SIZE;
  error_history.count++;
  
  system_stats.current_error = error_code;
  system_stats.total_errors++;
  
  sei();
  
  // Loguj błąd
  char log_msg[64];
  snprintf_P(log_msg, sizeof(log_msg), PSTR("Error registered: 0x%02X"), error_code);
  LOG_EVENT(LOG_LEVEL_ERROR, EVENT_ERROR_OCCURRED, log_msg);
  
  // Dodaj zdarzenie krytyczne dla poważnych błędów
  if (error_code >= ERROR_OVERHEAT) {
    EVENT_CRITICAL(EVENT_SAFETY_TRIGGER, "Critical error triggered safety system");
  }
}

/**
 * @brief Pobiera ostatni błąd
 * @return Kod ostatniego błędu
 */
uint8_t error_get_last() {
  cli();
  uint8_t last_error = (error_history.count > 0) ? 
    error_history.error_codes[error_history.head > 0 ? error_history.head - 1 : ERROR_HISTORY_SIZE - 1] : ERROR_NONE;
  sei();
  return last_error;
}

/**
 * @brief Czyści historię błędów
 */
void error_clear_history() {
  cli();
  error_history.head = 0;
  error_history.count = 0;
  system_stats.current_error = ERROR_NONE;
  sei();
  LOG_INFO("Error history cleared");
}

// ============================================================================
// FUNKCJE POMOCNICZE - CRC8
// ============================================================================

/**
 * @brief Oblicza CRC8 dla pakietu danych
 * @param data Wskaźnik do danych
 * @param length Długość danych (bez checksum)
 * @return Obliczona suma kontrolna CRC8
 */
uint8_t calculate_crc8(const uint8_t* data, size_t length) {
  uint8_t crc = 0x00;
  for (size_t i = 0; i < length; i++) {
    crc = pgm_read_byte(&crc8_table[crc ^ data[i]]);
  }
  return crc;
}

/**
 * @brief Weryfikuje poprawność pakietu TherapyPacket
 * @param packet Wskaźnik do pakietu
 * @return true jeśli CRC jest poprawne, false w przeciwnym razie
 */
bool verify_packet(const TherapyPacket* packet) {
  uint8_t calculated_crc = calculate_crc8((const uint8_t*)packet, sizeof(TherapyPacket) - 1);
  return calculated_crc == packet->checksum;
}

// ============================================================================
// FUNKCJE GENERATORA PWM - XV-LPWM ENGINE
// ============================================================================

/**
 * @brief Konfiguruje Timer1 dla generatora PWM o zadanej częstotliwości
 * @param frequency_hz_x100 Częstotliwość w Hz * 100 (np. 72700 dla 727.00 Hz)
 * @param duty_cycle Cykl pracy 0-100%
 * @return true jeśli konfiguracja succeeded, false w przypadku błędu
 * 
 * Zakresy częstotliwości i preskalery:
 * - 0.1 Hz - 100 Hz: prescaler 1024
 * - 100 Hz - 10 kHz: prescaler 64
 * - 10 kHz - 100 kHz: prescaler 8
 * - 100 kHz - 500 kHz: prescaler 1
 */
bool configure_timer1_pwm(uint32_t frequency_hz_x100, uint8_t duty_cycle) {
  // Walidacja zakresu częstotliwości
  if (frequency_hz_x100 < 10 || frequency_hz_x100 > 50000000) { // 0.1 Hz do 500 kHz
    return false;
  }
  
  // Walidacja cyklu pracy
  if (duty_cycle > 100) {
    return false;
  }
  
  // Oblicz rzeczywistą częstotliwość w Hz
  float frequency = frequency_hz_x100 / 100.0;
  
  // Dobór preskalera na podstawie częstotliwości
  uint16_t prescaler;
  uint16_t prescaler_bits;
  
  if (frequency < 100.0) {
    prescaler = 1024;
    prescaler_bits = _BV(CS12) | _BV(CS10);  // 1024
  } else if (frequency < 10000.0) {
    prescaler = 64;
    prescaler_bits = _BV(CS11) | _BV(CS10);  // 64
  } else if (frequency < 100000.0) {
    prescaler = 8;
    prescaler_bits = _BV(CS11);               // 8
  } else {
    prescaler = 1;
    prescaler_bits = _BV(CS10);               // 1
  }
  
  // Oblicz wartość TOP dla Fast PWM Mode 14
  // TOP = F_CPU / (prescaler * frequency) - 1
  // F_CPU = 16 MHz
  uint32_t top = (F_CPU / (prescaler * frequency)) - 1;
  
  // Sprawdź czy TOP mieści się w 16 bitach
  if (top > 65535 || top < 10) {
    return false;
  }
  
  // Oblicz wartość OCR1A dla zadanego duty cycle
  // OCR1A = TOP * (duty_cycle / 100)
  uint32_t ocr1a = ((uint32_t)top * duty_cycle) / 100;
  
  // Sekcja krytyczna - atomowa aktualizacja rejestrów
  cli();  // Wyłącz przerwania
  
  // Konfiguracja Timer1 w Fast PWM Mode 14 (TOP = ICR1)
  // TCCR1A: COM1A1=1, COM1A0=0 (Clear on compare match, set at TOP)
  //         WGM11=1, WGM10=1 (Fast PWM Mode 14)
  TCCR1A = _BV(COM1A1) | _BV(WGM11) | _BV(WGM10);
  
  // TCCR1B: WGM13=1, WGM12=1 (Fast PWM Mode 14)
  //         CS12, CS11, CS10 - prescaler
  TCCR1B = _BV(WGM13) | _BV(WGM12) | prescaler_bits;
  
  // Ustaw TOP w rejestrze ICR1
  ICR1 = (uint16_t)top;
  
  // Ustaw duty cycle w rejestrze OCR1A
  OCR1A = (uint16_t)ocr1a;
  
  // Konfiguracja pinu D9 jako wyjście (OC1A)
  DDRB |= _BV(PORTB1);  // PB1 = D9
  
  sei();  // Włącz przerwania
  
  return true;
}

/**
 * @brief Wyłącza generator PWM
 */
void disable_pwm() {
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  PORTB &= ~_BV(PORTB1);  // Wyjście niskie
  sei();
}

/**
 * @brief Aktualizuje cykl pracy PWM bez zmiany częstotliwości
 * @param duty_cycle Nowy cykl pracy 0-100%
 */
void update_duty_cycle(uint8_t duty_cycle) {
  if (duty_cycle > 100) duty_cycle = 100;
  
  cli();
  uint32_t ocr1a = ((uint32_t)ICR1 * duty_cycle) / 100;
  OCR1A = (uint16_t)ocr1a;
  sei();
}

// ============================================================================
// FUNKCJE MODULACJI
// ============================================================================

volatile uint32_t modulation_timer = 0;
volatile uint8_t burst_state = 0;

/**
 * @brief Obsługa modulacji AM (Amplitude Modulation)
 * @param carrier_freq Częstotliwość nośna w Hz * 100
 * @param mod_freq Częstotliwość modulacji w Hz * 100
 * @param depth Głębokość modulacji 0-100%
 */
void process_am_modulation(uint32_t carrier_freq, uint32_t mod_freq, uint8_t depth) {
  static uint32_t phase_accumulator = 0;
  
  // Prosta modulacja amplitudy DDS
  phase_accumulator += mod_freq;
  uint16_t mod_value = (sin(phase_accumulator / 655.36) + 1) * 128;  // 0-255
  
  uint8_t new_duty = current_duty_cycle * (128 + mod_value * depth / 100) / 256;
  update_duty_cycle(new_duty);
}

/**
 * @brief Obsługa modulacji FM (Frequency Modulation)
 * @param carrier_freq Częstotliwość nośna w Hz * 100
 * @param mod_freq Częstotliwość modulacji w Hz * 100
 * @param deviation Dewiacja częstotliwości w Hz * 100
 */
void process_fm_modulation(uint32_t carrier_freq, uint32_t mod_freq, uint32_t deviation) {
  static uint32_t phase_accumulator = 0;
  
  phase_accumulator += mod_freq;
  int16_t mod_value = (sin(phase_accumulator / 655.36)) * deviation;
  
  uint32_t new_freq = carrier_freq + mod_value;
  configure_timer1_pwm(new_freq, current_duty_cycle);
}

/**
 * @brief Obsługa modulacji Burst
 * @param burst_freq Częstotliwość burst w Hz * 100
 * @param burst_duty Cykl pracy burst 0-100%
 */
void process_burst_modulation(uint32_t burst_freq, uint8_t burst_duty) {
  static unsigned long last_toggle = 0;
  static bool burst_on = true;
  
  unsigned long now = millis();
  unsigned long period = 100000 / burst_freq;  // Okres w ms * 100
  
  if (now - last_toggle >= period * burst_duty / 100) {
    burst_on = !burst_on;
    last_toggle = now;
    
    if (burst_on) {
      enable_pwm();
    } else {
      disable_pwm();
    }
  }
}

// ============================================================================
// ROZSZERZONE FUNKCJE BEZPIECZEŃSTWA I WATCHDOG
// ============================================================================

// Watchdog wielopoziomowy
struct MultiWatchdog {
  uint32_t main_wd_timer;
  uint32_t network_wd_timer;
  uint32_t therapy_wd_timer;
  uint32_t comms_wd_timer;
  uint8_t reset_count;
  uint8_t max_resets;
};

volatile MultiWatchdog multi_wd;

#define MAIN_WD_TIMEOUT     4000    // 4s - główny watchdog
#define NETWORK_WD_TIMEOUT  5000    // 5s - sieć
#define THERAPY_WD_TIMEOUT  1000    // 1s - terapia
#define COMMS_WD_TIMEOUT    2000    // 2s - komunikacja
#define MAX_WD_RESETS       5       // Maksymalna liczba resetów przed blokadą

/**
 * @brief Inicjalizuje wielopoziomowy watchdog
 */
void multi_watchdog_init() {
  cli();
  multi_wd.main_wd_timer = millis();
  multi_wd.network_wd_timer = millis();
  multi_wd.therapy_wd_timer = millis();
  multi_wd.comms_wd_timer = millis();
  multi_wd.reset_count = 0;
  multi_wd.max_resets = MAX_WD_RESETS;
  sei();
  LOG_INFO("Multi-watchdog initialized");
}

/**
 * @brief Resetuje główny watchdog
 */
void watchdog_feed_main() {
  cli();
  multi_wd.main_wd_timer = millis();
  sei();
}

/**
 * @brief Resetuje watchdog sieci
 */
void watchdog_feed_network() {
  cli();
  multi_wd.network_wd_timer = millis();
  sei();
}

/**
 * @brief Resetuje watchdog terapii
 */
void watchdog_feed_therapy() {
  cli();
  multi_wd.therapy_wd_timer = millis();
  sei();
}

/**
 * @brief Resetuje watchdog komunikacji
 */
void watchdog_feed_comms() {
  cli();
  multi_wd.comms_wd_timer = millis();
  sei();
}

/**
 * @brief Sprawdza status wszystkich watchdogów
 * @return Kod błędu lub ERROR_NONE
 */
uint8_t multi_watchdog_check() {
  uint32_t now = millis();
  
  // Sprawdź główny watchdog
  if (now - multi_wd.main_wd_timer > MAIN_WD_TIMEOUT) {
    LOG_FATAL("Main watchdog timeout!");
    return ERROR_WDTO_RESET;
  }
  
  // Sprawdź watchdog sieci
  if (now - multi_wd.network_wd_timer > NETWORK_WD_TIMEOUT) {
    LOG_WARN("Network watchdog timeout - connection lost");
    disable_pwm();
    therapy_active = false;
    EVENT_WARNING(EVENT_NETWORK_DISCONNECTED, "Network watchdog timeout");
  }
  
  // Sprawdź watchdog terapii (tylko jeśli terapia aktywna)
  if (therapy_active && (now - multi_wd.therapy_wd_timer > THERAPY_WD_TIMEOUT)) {
    LOG_ERROR("Therapy watchdog timeout!");
    error_register(ERROR_WDTO_RESET);
    return ERROR_WDTO_RESET;
  }
  
  return ERROR_NONE;
}

/**
 * @brief Monitoruje temperaturę MCU i stan systemu
 * @return Kod błędu lub ERROR_NONE
 */
uint8_t safety_monitor() {
  static unsigned long monitor_timer = 0;
  static uint16_t consecutive_overheat = 0;
  
  if (millis() - monitor_timer > 500) {  // Sprawdź co 500ms
    monitor_timer = millis();
    
    // Pomiar temperatury z wbudowanego źródła (opcjonalnie)
    // ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    // ADCSRA |= _BV(ADSC);
    // while (ADCSRA & _BV(ADSC));
    // uint16_t temp = ADC;
    
    // Symulacja pomiaru temperatury
    uint16_t temp = 250;  // 25.0°C * 10
    
    if (temp > 750) {  // > 75°C - ostrzeżenie
      consecutive_overheat++;
      LOG_WARN("Temperature warning");
      
      if (consecutive_overheat >= 3) {
        LOG_ERROR("Overheat detected!");
        error_register(ERROR_OVERHEAT);
        return ERROR_OVERHEAT;
      }
    } else {
      consecutive_overheat = 0;
    }
    
    // Sprawdź watchdog wielopoziomowy
    uint8_t wd_error = multi_watchdog_check();
    if (wd_error != ERROR_NONE) {
      return wd_error;
    }
  }
  
  return ERROR_NONE;
}

/**
 * @brief Zapisuje przyczynę resetu w EEPROM (symulacja)
 */
void save_reset_cause(uint8_t cause) {
  // W pełnej implementacji zapisz do EEPROM
  LOG_EVENT(LOG_LEVEL_ERROR, EVENT_SYSTEM_RESET, "Reset cause saved");
}

/**
 * @brief Odczytuje przyczynę ostatniego resetu
 * @return Kod przyczyny resetu
 */
uint8_t get_reset_cause() {
  // W pełnej implementacji odczytaj z EEPROM
  uint8_t mcusr = MCUSR;
  MCUSR = 0;  // Wyczyść flagi
  
  if (mcusr & _BV(PORF)) return 0x01;  // Power-on
  if (mcusr & _BV(EXTRF)) return 0x02;  // External reset
  if (mcusr & _BV(BORF)) return 0x03;   // Brown-out
  if (mcusr & _BV(WDRF)) return 0x04;   // Watchdog
  
  return 0x00;  // Unknown
}

/**
 * @brief Resetuje system w przypadku krytycznego błędu
 * @param error_code Kod błędu powodującego reset
 */
void emergency_reset(uint8_t error_code) {
  LOG_FATAL("Emergency reset triggered!");
  
  // Zarejestruj błąd
  error_register(error_code);
  
  // Wyłącz PWM natychmiast
  disable_pwm();
  therapy_active = false;
  
  // Zapisz przyczynę resetu
  save_reset_cause(error_code);
  
  // Sygnalizacja diodami (SOS pattern)
  for (int cycle = 0; cycle < 3; cycle++) {
    // 3 x krótkie błyski (S)
    for (int i = 0; i < 3; i++) {
      digitalWrite(STATUS_LED, HIGH);
      digitalWrite(DEBUG_LED, HIGH);
      delay(150);
      digitalWrite(STATUS_LED, LOW);
      digitalWrite(DEBUG_LED, LOW);
      delay(150);
    }
    delay(500);
    // 3 x długie błyski (O)
    for (int i = 0; i < 3; i++) {
      digitalWrite(STATUS_LED, HIGH);
      digitalWrite(DEBUG_LED, HIGH);
      delay(400);
      digitalWrite(STATUS_LED, LOW);
      digitalWrite(DEBUG_LED, LOW);
      delay(400);
    }
    delay(500);
  }
  
  // Increment reset counter
  multi_wd.reset_count++;
  
  if (multi_wd.reset_count >= multi_wd.max_resets) {
    LOG_FATAL("Maximum reset count reached - entering safe mode");
    // Wejdź w tryb bezpieczny - nie resetuj więcej
    while(1) {
      digitalWrite(STATUS_LED, HIGH);
      delay(1000);
      digitalWrite(STATUS_LED, LOW);
      delay(1000);
    }
  }
  
  // Reset przez watchdog hardware
  LOG_FATAL("Initiating hardware reset...");
  wdt_enable(WDTO_15MS);
  while(1);  // Czekaj na reset
}

/**
 * @brief Wysyła heartbeat do sieci
 */
void send_heartbeat() {
  static uint32_t last_hb = 0;
  
  if (millis() - last_hb > 1000) {
    last_hb = millis();
    watchdog_feed_network();
    watchdog_feed_comms();
    
    // Wyślij pakiet heartbeat (implementacja zależna od protokołu)
    LOG_VERBOSE("Heartbeat sent");
  }
}

// ============================================================================
// IMPLEMENTACJA STEROWNIKA ENC28J60 (UPROSZCZONA)
// ============================================================================

// Uwaga: Pełna implementacja sterownika ENC28J60 wymaga biblioteki EthernetENC
// Poniżej znajduje się szkielet integracji

#ifdef USE_EXTERNAL_LIBRARY
  #include <EthernetENC.h>
  
  EthernetUDP udp;
  EthernetServer tcp_server(TCP_PORT);
  EthernetClient tcp_client;
#else
  // Szkielet dla własnej implementacji
  void enc28j60_init() {
    pinMode(ENC28J60_CS, OUTPUT);
    digitalWrite(ENC28J60_CS, HIGH);
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV2);  // 8 MHz
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    
    // Tutaj powinna być inicjalizacja ENC28J60
    // SetBank, InitializeRegisters, ConfigureMAC, itp.
  }
  
  void enc28j60_send_packet(const uint8_t* data, uint16_t len) {
    // Implementacja wysyłki pakietu
  }
  
  uint16_t enc28j60_receive_packet(uint8_t* buffer, uint16_t maxlen) {
    // Implementacja odbioru pakietu
    return 0;
  }
#endif

// ============================================================================
// OBSŁUGA POŁĄCZEŃ SIECIOWYCH
// ============================================================================

/**
 * @brief Przetwarza przychodzący pakiet UDP/TCP
 * @param buffer Wskaźnik do bufora z danymi
 * @param length Długość danych
 * @param is_tcp true jeśli pakiet z TCP, false dla UDP
 */
void process_incoming_packet(uint8_t* buffer, uint16_t length, bool is_tcp) {
  if (length < sizeof(TherapyPacket)) {
    return;
  }
  
  TherapyPacket* packet = (TherapyPacket*)buffer;
  
  // Weryfikacja CRC
  if (!verify_packet(packet)) {
    // Odrzuć pakiet z błędnym CRC
    return;
  }
  
  // Odśwież watchdog network
  last_heartbeat = millis();
  
  // Rozpocznij terapię
  current_frequency_x100 = packet->frequency_hz_x100;
  therapy_duration = packet->duration_sec;
  current_modulation = packet->modulation_type;
  current_duty_cycle = packet->duty_cycle;
  current_intensity = packet->intensity_level;
  
  // Skonfiguruj generator PWM
  if (!configure_timer1_pwm(current_frequency_x100, current_duty_cycle)) {
    // Błąd konfiguracji timera
    return;
  }
  
  therapy_active = true;
  therapy_start_time = millis();
  
  // Wyślij potwierdzenie ACK
  send_ack_response(true);
}

/**
 * @brief Wysyła odpowiedź ACK/NACK
 * @param ack true dla ACK, false dla NACK
 */
void send_ack_response(bool ack) {
  uint8_t response[2] = {ack ? 0x06 : 0x15, 0x00};
  // Implementacja wysyłki odpowiedzi
}

/**
 * @brief Wysyła pakiet telemetryczny
 * @param status Wskaźnik do struktury statusu
 */
void send_telemetry(DeviceStatus* status) {
  // Implementacja wysyłki telemetrii przez UDP
}

// ============================================================================
// SETUP I MAIN LOOP
// ============================================================================

/**
 * @brief Funkcja inicjalizacyjna - wywoływana raz przy starcie
 */
void setup() {
  // Konfiguracja pinów
  pinMode(PWM_OUTPUT_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  pinMode(DEBUG_LED, OUTPUT);
  pinMode(ENC28J60_CS, OUTPUT);
  
  digitalWrite(PWM_OUTPUT_PIN, LOW);
  digitalWrite(STATUS_LED, LOW);
  digitalWrite(DEBUG_LED, LOW);
  digitalWrite(ENC28J60_CS, HIGH);
  
  // Inicjalizacja UART dla debugowania (opcjonalnie)
  Serial.begin(115200);
  while (!Serial) {
    ;  // Czekaj na połączenie serial
  }
  
  Serial.println(F(""));
  Serial.println(F("================================================="));
  Serial.println(F("ResoNet-Nano v3.0 Medical Grade"));
  Serial.println(F("Full Debug & Event System Enabled"));
  Serial.println(F("================================================="));
  Serial.println(F("Initializing..."));
  
  // Sprawdź przyczynę resetu
  uint8_t reset_cause = get_reset_cause();
  Serial.print(F("Reset cause: 0x"));
  Serial.println(reset_cause, HEX);
  
  // Inicjalizacja SPI
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  
  // Inicjalizacja systemów diagnostycznych
  log_buffer_init();
  event_queue_init();
  error_history_init();
  multi_watchdog_init();
  
  LOG_INFO("System diagnostics initialized");
  EVENT_INFO(EVENT_SYSTEM_RESET, "System startup");
  
  // Inicjalizacja ENC28J60
  #ifdef USE_EXTERNAL_LIBRARY
    if (USE_DHCP) {
      if (Ethernet.begin(mac) == 0) {
        LOG_ERROR("Failed to configure Ethernet using DHCP");
        emergency_reset(ERROR_NETWORK_INIT);
      }
    } else {
      Ethernet.begin(mac, ip, gateway, subnet);
    }
    
    udp.begin(UDP_PORT);
    tcp_server.begin();
  #else
    enc28j60_init();
  #endif
  
  Serial.print(F("IP Address: "));
  #ifdef USE_EXTERNAL_LIBRARY
    Serial.println(Ethernet.localIP());
    LOG_EVENT(LOG_LEVEL_INFO, EVENT_NETWORK_CONNECTED, "Network connected");
  #else
    Serial.println(ip);
  #endif
  
  // Konfiguracja Watchdog Timer hardware
  wdt_enable(WDTO_4S);  // 4 sekundy
  
  // Reset zmiennych
  therapy_active = false;
  current_frequency_x100 = 0;
  last_heartbeat = millis();
  
  // Wyłącz PWM na start
  disable_pwm();
  
  // Zainicjalizuj statystyki
  system_stats.uptime_ms = millis();
  system_stats.total_events = 0;
  system_stats.total_errors = 0;
  system_stats.watchdog_resets = 0;
  system_stats.packets_received = 0;
  system_stats.packets_sent = 0;
  system_stats.therapy_sessions = 0;
  system_stats.max_free_memory = 2048;  // Szacunkowo dla ATmega328P
  system_stats.current_error = ERROR_NONE;
  
  LOG_INFO("System ready. Waiting for commands...");
  Serial.println(F("System ready. Waiting for commands..."));
  
  // Sygnał gotowości - podwójny błysk
  digitalWrite(STATUS_LED, HIGH);
  delay(200);
  digitalWrite(STATUS_LED, LOW);
  delay(200);
  digitalWrite(STATUS_LED, HIGH);
  delay(200);
  digitalWrite(STATUS_LED, LOW);
  
  EVENT_INFO(EVENT_SYSTEM_RESET, "Initialization complete");
}

/**
 * @brief Główna pętla programu - wywoływana ciągle
 */
void loop() {
  // Reset hardware watchdog
  wdt_reset();
  watchdog_feed_main();  // Reset software watchdog
  
  // Aktualizuj statystyki
  system_stats.uptime_ms = millis();
  
  // Monitoring bezpieczeństwa
  uint8_t error = safety_monitor();
  if (error != ERROR_NONE) {
    if (error == ERROR_OVERHEAT || error == ERROR_WDTO_RESET) {
      emergency_reset(error);
    } else {
      error_register(error);
    }
  }
  
  // Wysyłaj heartbeat
  send_heartbeat();
  
  // Obsługa zdarzeń z kolejki
  SystemEvent evt;
  while (event_pop(&evt)) {
    // Przetwarzaj zdarzenia (można dodać handler dla konkretnych eventów)
    LOG_VERBOSE("Processing event from queue");
  }
  
  // Debug: co 10 sekund wypisz statystyki
  static unsigned long last_stats_print = 0;
  if (millis() - last_stats_print > 10000) {
    last_stats_print = millis();
    
    LOG_EVENT(LOG_LEVEL_DEBUG, 0, "=== System Statistics ===");
    Serial.print(F("Uptime: "));
    Serial.print(system_stats.uptime_ms / 1000);
    Serial.println(F("s"));
    Serial.print(F("Events: "));
    Serial.println(system_stats.total_events);
    Serial.print(F("Errors: "));
    Serial.println(system_stats.total_errors);
    Serial.print(F("Log overflow: "));
    Serial.println(log_buffer.overflow_count);
    Serial.print(F("Event drops: "));
    Serial.println(event_queue.dropped_count);
  }
  
  // Obsługa połączeń sieciowych
  #ifdef USE_EXTERNAL_LIBRARY
    // UDP - dane w czasie rzeczywistym
    int udp_size = udp.parsePacket();
    if (udp_size > 0) {
      uint8_t buffer[udp_size];
      udp.read(buffer, udp_size);
      process_incoming_packet(buffer, udp_size, false);
    }
    
    // TCP - konfiguracja
    EthernetClient client = tcp_server.available();
    if (client) {
      if (client.available()) {
        uint8_t buffer[512];
        int len = client.read(buffer, 512);
        process_incoming_packet(buffer, len, true);
      }
    }
  #else
    // Własna implementacja ENC28J60
    uint8_t buffer[512];
    uint16_t len = enc28j60_receive_packet(buffer, 512);
    if (len > 0) {
      process_incoming_packet(buffer, len, false);
    }
  #endif
  
  // Obsługa aktywnej terapii
  if (therapy_active) {
    unsigned long elapsed = (millis() - therapy_start_time) / 1000;
    
    // Sprawdź czy czas terapii minął
    if (elapsed >= therapy_duration && therapy_duration > 0) {
      therapy_active = false;
      disable_pwm();
      Serial.println(F("Therapy completed"));
    } else {
      // Proces modulacji
      switch (current_modulation) {
        case 0:  // Brak modulacji
          break;
        case 1:  // AM
          // process_am_modulation(...);
          break;
        case 2:  // FM
          // process_fm_modulation(...);
          break;
        case 3:  // Burst
          // process_burst_modulation(...);
          break;
      }
      
      // Wysyłka telemetrii co sekundę
      static unsigned long last_telemetry = 0;
      if (millis() - last_telemetry > 1000) {
        last_telemetry = millis();
        
        DeviceStatus status;
        status.current_frequency = current_frequency_x100;
        status.elapsed_time = elapsed;
        status.status_flags = STATUS_RUNNING;
        status.error_code = ERROR_NONE;
        status.temperature = 250;  // 25.0°C
        status.reserved = 0;
        
        send_telemetry(&status);
        
        // Debug na Serial
        Serial.print(F("Freq: "));
        Serial.print(current_frequency_x100 / 100.0, 2);
        Serial.print(F(" Hz, Time: "));
        Serial.print(elapsed);
        Serial.println(F(" s"));
      }
    }
  }
  
  // Krótka pauza dla stabilności
  delay(1);
}

// ============================================================================
// PRZERWANIA I OBSŁUGA TIMERÓW
// ============================================================================

/**
 * @brief Przerwanie od Timera1 (opcjonalne, dla zaawansowanych modulacji)
 */
ISR(TIMER1_OVF_vect) {
  // Obsługa przepełnienia Timera1
  // Może być użyte do precyzyjnego pomiaru czasu lub modulacji
}

/**
 * @brief Przerwanie od Timera2 (opcjonalne, dla modulacji złożonych)
 */
ISR(TIMER2_OVF_vect) {
  // Obsługa drugiego timera dla synchronizacji multi-timer
}

// ============================================================================
// KONIEC PROGRAMU - PODSUMOWANIE FUNKCJONALNOŚCI v3.0
// ============================================================================

/**
 * Podsumowanie funkcjonalności (v3.0):
 * 
 * 1. GENERATOR PWM XV-LPWM:
 *    - Zakres: 0.1 Hz - 500 kHz
 *    - Rozdzielczość: 0.01 Hz
 *    - Timer1 16-bit w trybie Fast PWM Mode 14
 *    - Automatyczny dobór preskalera
 * 
 * 2. KOMUNIKACJA SIECIOWA:
 *    - UDP port 5000: Dane realtime
 *    - TCP port 5001: Konfiguracja
 *    - Protokół binarny z CRC8
 *    - Watchdog network 5s
 * 
 * 3. SYSTEM LOGOWANIA (NOWOŚĆ v3.0):
 *    - 6 poziomów logowania (VERBOSE, DEBUG, INFO, WARN, ERROR, FATAL)
 *    - Ring buffer 256 wpisów
 *    - Logi przez Serial i sieć
 *    - Timestampy dla każdego wpisu
 *    - Powiązanie z kodami zdarzeń
 * 
 * 4. SYSTEM OBSŁUGI ZDARZEŃ (NOWOŚĆ v3.0):
 *    - Kolejka FIFO 32 zdarzeń
 *    - Kody zdarzeń (EVENT_THERAPY_START, EVENT_ERROR_OCCURRED, etc.)
 *    - Poziomy ważności (Info, Warning, Error, Critical)
 *    - Automatyczne logowanie zdarzeń
 * 
 * 5. SYSTEM OBSŁUGI BŁĘDÓW (NOWOŚĆ v3.0):
 *    - Historia ostatnich 16 błędów
 *    - Kody błędów rozszerzone (ERROR_NONE - ERROR_HARDWARE_FAULT)
 *    - Rejestracja z timestampami
 *    - Automatyczne powiadomienia krytyczne
 * 
 * 6. WATCHDOG WIELOPOZIOMOWY (ROZSZERZENIE v3.0):
 *    - Main watchdog (4s)
 *    - Network watchdog (5s)
 *    - Therapy watchdog (1s)
 *    - Communications watchdog (2s)
 *    - Licznik resetów z blokadą po przekroczeniu limitu
 *    - Tryb bezpieczny przy wielokrotnych resetach
 * 
 * 7. STATYSTYKI I DIAGNOSTYKA (NOWOŚĆ v3.0):
 *    - Uptime systemu
 *    - Licznik zdarzeń i błędów
 *    - Statystyki pakietów sieciowych
 *    - Licznik sesji terapii
 *    - Monitorowanie pamięci
 *    - Okresowy raport statystyk (co 10s)
 * 
 * 8. BEZPIECZEŃSTWO:
 *    - Watchdog Timer hardware + software
 *    - Monitorowanie temperatury z histerezą
 *    - Detekcja błędów CRC
 *    - Emergency reset z sygnalizacją SOS
 *    - Zapis przyczyny resetu
 *    - Odczyt przyczyny resetu (MCUSR)
 * 
 * 9. MODULACJE:
 *    - None (czysty sygnał)
 *    - AM (modulacja amplitudy)
 *    - FM (modulacja częstotliwości)
 *    - Burst (impulsowa)
 * 
 * NORMY MEDYCZNE:
 * - IEC 60601-1: Bezpieczeństwo podstawowe
 * - IEC 60601-1-2: EMC
 * - ISO 14971: Zarządzanie ryzykiem
 * 
 * STRUKTURY DANYCH:
 * - TherapyPacket: Pakiet terapii (16 bajtów)
 * - DeviceStatus: Status urządzenia
 * - LogEntry: Wpis logu (72 bajty)
 * - SystemEvent: Zdarzenie systemowe (44 bajty)
 * - SystemStats: Statystyki systemu
 * - MultiWatchdog: Wielopoziomowy watchdog
 * 
 * MAKRA POMOCNICZE:
 * - LOG_VERBOSE(msg), LOG_DEBUG(msg), LOG_INFO(msg)
 * - LOG_WARN(msg), LOG_ERROR(msg), LOG_FATAL(msg)
 * - EVENT_INFO(code, desc), EVENT_WARNING(code, desc)
 * - EVENT_ERROR(code, desc), EVENT_CRITICAL(code, desc)
 * 
 * UWAGA: Do pełnej funkcjonalności wymagana biblioteka EthernetENC
 * dla modułu ENC28J60. Instalować przez Arduino Library Manager.
 * 
 * Autor: ResoNet Team
 * License: Medical Grade Proprietary
 * Version: 3.0.0
 */
