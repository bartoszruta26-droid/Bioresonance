/**
 * @file ResoNet_Nano.ino
 * @brief Profesjonalny System Biorezonansu Klasy Medycznej - Główny Plik Arduino
 * @version 4.0 (Modular Architecture with Pseudo-threading)
 * 
 * @description
 * Architektura modularna z pseudowielowątkowością realizowaną przez:
 * - Kooperacyjne wielozadaniowość (cooperative multitasking)
 * - Maszyny stanów dla każdego modułu
 * - Non-blocking timing z millis()
 * 
 * Moduły systemu:
 * - safety_system: Bezpieczeństwo medyczne IEC 60601-1
 * - logging_system: Logowanie z ring bufferem
 * - event_system: Obsługa zdarzeń FIFO
 * - pwm_engine: Generator sygnałów PWM
 * - network_system: Komunikacja Ethernet ENC28J60
 * 
 * BEZ PYTHONA - 100% C++
 */

#include <SPI.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

// ============================================================================
// INCLUDE MODUŁÓW
// ============================================================================

#include "safety_system.h"
#include "logging_system.h"
#include "event_system.h"
#include "pwm_engine.h"
#include "network_system.h"
#include "ir_led_engine.h"  // Nowy moduł: IR LED Strip
#include "device_detector.h"  // Nowy moduł: Wykrywanie efektorów i sensorów

// ============================================================================
// KONFIGURACJA SYSTEMU
// ============================================================================

#define STATUS_LED 13
#define DEBUG_LED 8

// Interwały czasowe dla pseudowielowątkowości (w ms)
#define TASK_SAFETY_INTERVAL      50    // Co 50ms
#define TASK_LOGGING_INTERVAL     100   // Co 100ms
#define TASK_EVENT_INTERVAL       50    // Co 50ms
#define TASK_PWM_INTERVAL         10    // Co 10ms
#define TASK_IR_LED_INTERVAL      10    // Co 10ms (IR LED Strip)
#define TASK_DEVICE_INTERVAL      100   // Co 100ms (Device Detection)
#define TASK_NETWORK_INTERVAL     100   // Co 100ms
#define TASK_STATS_INTERVAL       10000 // Co 10s

// ============================================================================
// STRUKTURY STANÓW ZADAŃ (PSEUDOWĄTKI)
// ============================================================================

typedef struct {
    uint32_t last_run;
    uint32_t interval;
    uint32_t execution_count;
    uint32_t max_execution_time;
    bool enabled;
} TaskControl;

// Kontrolery zadań
static TaskControl task_safety = {0, TASK_SAFETY_INTERVAL, 0, 0, true};
static TaskControl task_logging = {0, TASK_LOGGING_INTERVAL, 0, 0, true};
static TaskControl task_events = {0, TASK_EVENT_INTERVAL, 0, 0, true};
static TaskControl task_pwm = {0, TASK_PWM_INTERVAL, 0, 0, true};
static TaskControl task_ir_led = {0, TASK_IR_LED_INTERVAL, 0, 0, true};  // IR LED Strip
static TaskControl task_device = {0, TASK_DEVICE_INTERVAL, 0, 0, true};  // Device Detection
static TaskControl task_network = {0, TASK_NETWORK_INTERVAL, 0, 0, true};
static TaskControl task_stats = {0, TASK_STATS_INTERVAL, 0, 0, true};

// Statystyki systemowe
typedef struct {
    uint32_t uptime_ms;
    uint32_t loop_count;
    uint32_t total_tasks_executed;
    uint16_t free_memory;
    uint8_t error_count;
} SystemStats;

static SystemStats stats;

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

/**
 * @brief Oblicza dostępną pamięć RAM
 */
static int freeMemory() {
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

/**
 * @brief Sprawdza czy zadanie powinno być wykonane
 */
static bool should_run_task(TaskControl* task, uint32_t now) {
    if (!task->enabled) return false;
    if (now - task->last_run >= task->interval) {
        task->last_run = now;
        task->execution_count++;
        return true;
    }
    return false;
}

/**
 * @brief Mierzy czas wykonania zadania
 */
static uint32_t measure_task_start() {
    return micros();
}

static void measure_task_end(TaskControl* task, uint32_t start) {
    uint32_t elapsed = micros() - start;
    if (elapsed > task->max_execution_time) {
        task->max_execution_time = elapsed;
    }
}

// ============================================================================
// IMPLEMENTACJA ZADAŃ (PSEUDOWĄTKI)
// ============================================================================

/**
 * @brief Zadanie bezpieczeństwa (kritczne!)
 */
static void task_safety_run() {
    uint32_t start = measure_task_start();
    
    safety_loop();
    
    // Feed watchdog dla warstwy głównej
    safety_feed(WDT_LAYER_MAIN);
    
    measure_task_end(&task_safety, start);
}

/**
 * @brief Zadanie logowania
 */
static void task_logging_run() {
    uint32_t start = measure_task_start();
    
    logging_loop();
    
    measure_task_end(&task_logging, start);
}

/**
 * @brief Zadanie obsługi zdarzeń
 */
static void task_events_run() {
    uint32_t start = measure_task_start();
    
    event_system_loop();
    
    measure_task_end(&task_events, start);
}

/**
 * @brief Zadanie silnika PWM
 */
static void task_pwm_run() {
    uint32_t start = measure_task_start();
    
    pwm_loop();
    
    // Feed watchdog dla warstwy terapii jeśli PWM działa
    if (pwm_is_running()) {
        safety_feed(WDT_LAYER_THERAPY);
    }
    
    measure_task_end(&task_pwm, start);
}

/**
 * @brief Zadanie silnika IR LED Strip
 */
static void task_ir_led_run() {
    uint32_t start = measure_task_start();
    
    ir_led_loop();
    
    // Feed watchdog dla warstwy terapii jeśli IR LED działa
    if (ir_led_is_running()) {
        safety_feed(WDT_LAYER_THERAPY);
    }
    
    measure_task_end(&task_ir_led, start);
}

/**
 * @brief Zadanie wykrywania urządzeń (efektory i sensory)
 */
static void task_device_run() {
    uint32_t start = measure_task_start();
    
    device_detector_loop();
    
    measure_task_end(&task_device, start);
}

/**
 * @brief Zadanie sieciowe
 */
static void task_network_run() {
    uint32_t start = measure_task_start();
    
    network_loop();
    
    // Feed watchdog dla warstwy komunikacji
    if (network_is_connected()) {
        safety_feed(WDT_LAYER_COMMS);
        safety_feed(WDT_LAYER_NETWORK);
    }
    
    measure_task_end(&task_network, start);
}

/**
 * @brief Zadanie statystyk i diagnostyki
 */
static void task_stats_run() {
    uint32_t start = measure_task_start();
    
    stats.uptime_ms = millis();
    stats.free_memory = freeMemory();
    
    // Loguj statystyki
    LOG_INFO_F("Stats: Uptime=%lus, Loops=%lu, Tasks=%lu, FreeMem=%d",
               stats.uptime_ms / 1000,
               stats.loop_count,
               stats.total_tasks_executed,
               stats.free_memory);
    
    // Sprawdź pamięć
    if (stats.free_memory < 200) {
        LOG_ERROR("Low memory warning!");
        stats.error_count++;
    }
    
    measure_task_end(&task_stats, start);
}

// ============================================================================
// ARDUINO SETUP I LOOP
// ============================================================================

/**
 * @brief Inicjalizacja systemu
 */
void setup() {
    // Inicjalizacja Serial
    Serial.begin(115200);
    while (!Serial) {
        ;  // Czekaj na połączenie USB
    }
    
    delay(1000);  // Stabilizacja
    
    Serial.println(F("\n=== ResoNet Nano v4.0 ==="));
    Serial.println(F("Medical Grade Bioresonance System"));
    Serial.println(F("Modular Architecture with Pseudo-threading\n"));
    
    // Konfiguracja pinów
    pinMode(STATUS_LED, OUTPUT);
    pinMode(DEBUG_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);
    digitalWrite(DEBUG_LED, LOW);
    
    // Inicjalizacja modułów w kolejności od najbezpieczniejszych
    LOG_INFO("System startup initiated");
    
    // 1. System logowania (najpierw żeby móc logować błędy)
    logging_init();
    
    // 2. System zdarzeń
    event_system_init();
    
    // 3. System bezpieczeństwa
    safety_init();
    
    // Sprawdź czy jesteśmy w bezpiecznym stanie
    if (!SAFE_IS_SAFE()) {
        LOG_FATAL("Safety system in unsafe state - aborting startup");
        Serial.println(F("FATAL: Safety system fault!"));
        
        // Migaj diodą błędu
        while (true) {
            digitalWrite(STATUS_LED, HIGH);
            delay(200);
            digitalWrite(STATUS_LED, LOW);
            delay(200);
        }
    }
    
    // 4. Silnik PWM
    pwm_init();
    
    // 5. System sieciowy
    network_init();
    
    // 6. System detekcji urządzeń
    device_detector_init();
    
    // Inicjalizacja statystyk
    memset(&stats, 0, sizeof(SystemStats));
    stats.free_memory = freeMemory();
    
    LOG_INFO_F("System initialized successfully. Free memory: %d bytes", stats.free_memory);
    EVENT_INFO(EVENT_SYSTEM_RESET, "System startup complete");
    
    // Rozmieść diodę statusu
    digitalWrite(STATUS_LED, HIGH);
    delay(500);
    digitalWrite(STATUS_LED, LOW);
    
    Serial.println(F("System ready. Waiting for commands..."));
}

/**
 * @brief Główna pętla z pseudowielowątkowością
 * 
 * Wszystkie zadania są wykonywane kooperacyjnie w jednej pętli.
 * Każde zadanie ma swój interwał i jest wywoływane tylko gdy minie odpowiedni czas.
 * To zapewnia deterministyczne czasy reakcji bez blokowania innych zadań.
 */
void loop() {
    uint32_t now = millis();
    
    // Aktualizuj licznik pętli
    stats.loop_count++;
    
    // ================================================================
    // WYKONAJ ZADANIA KTÓRE SĄ GOTOWE (PSEUDOWIELOWĄTKOWOŚĆ)
    // ================================================================
    
    // Zadanie bezpieczeństwa - najwyższy priorytet
    if (should_run_task(&task_safety, now)) {
        task_safety_run();
        stats.total_tasks_executed++;
    }
    
    // Zadanie PWM - wysoki priorytet (timing krytyczny)
    if (should_run_task(&task_pwm, now)) {
        task_pwm_run();
        stats.total_tasks_executed++;
    }
    
    // Zadanie IR LED Strip - wysoki priorytet (timing krytyczny)
    if (should_run_task(&task_ir_led, now)) {
        task_ir_led_run();
        stats.total_tasks_executed++;
    }
    
    // Zadanie wykrywania urządzeń
    if (should_run_task(&task_device, now)) {
        task_device_run();
        stats.total_tasks_executed++;
    }
    
    // Zadanie zdarzeń
    if (should_run_task(&task_events, now)) {
        task_events_run();
        stats.total_tasks_executed++;
    }
    
    // Zadanie logowania
    if (should_run_task(&task_logging, now)) {
        task_logging_run();
        stats.total_tasks_executed++;
    }
    
    // Zadanie sieciowe
    if (should_run_task(&task_network, now)) {
        task_network_run();
        stats.total_tasks_executed++;
    }
    
    // Zadanie statystyk - niski priorytet
    if (should_run_task(&task_stats, now)) {
        task_stats_run();
        stats.total_tasks_executed++;
    }
    
    // ================================================================
    // OBSŁUGA WEJŚCIA SZEREGOWEGO (DEBUG/KOMENDY)
    // ================================================================
    
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case 's':
                // Status
                Serial.println(F("\n=== System Status ==="));
                Serial.print(F("Uptime: "));
                Serial.print(millis() / 1000);
                Serial.println(F("s"));
                Serial.print(F("Free Memory: "));
                Serial.print(stats.free_memory);
                Serial.println(F(" bytes"));
                Serial.print(F("Loop Count: "));
                Serial.println(stats.loop_count);
                Serial.print(F("Tasks Executed: "));
                Serial.println(stats.total_tasks_executed);
                
                SafetyStatus* safe = safety_get_status();
                Serial.print(F("Safety State: "));
                Serial.println(safe->safe_state);
                Serial.print(F("Temperature: "));
                Serial.print(safe->temperature_x10 / 10);
                Serial.println(F(" C"));
                
                PWMStatus* pwm = pwm_get_status();
                Serial.print(F("PWM Running: "));
                Serial.println(pwm->is_running ? "YES" : "NO");
                if (pwm->is_running) {
                    Serial.print(F("Frequency: "));
                    Serial.print(pwm->current_freq_x100 / 100);
                    Serial.print(F("."));
                    Serial.print(pwm->current_freq_x100 % 100);
                    Serial.println(F(" Hz"));
                }
                
                Serial.println(F("Network: "));
                Serial.println(network_is_connected() ? "CONNECTED" : "DISCONNECTED");
                
                // Device status
                DeviceSystemState_t* dev = get_device_system_state();
                Serial.print(F("Effector: "));
                switch (dev->effector.type) {
                    case EFFECTOR_HELMHOLTZ: Serial.println(F("Helmholtz Coil")); break;
                    case EFFECTOR_OTIC: Serial.println(F("Otic Applicator")); break;
                    case EFFECTOR_CONTACT: Serial.println(F("Contact Electrodes")); break;
                    case EFFECTOR_WRAP: Serial.println(F("Wrap Applicator")); break;
                    default: Serial.println(F("None")); break;
                }
                Serial.print(F("Connected: "));
                Serial.println(dev->effector.connected ? "YES" : "NO");
                Serial.print(F("Biofeedback: "));
                Serial.println(dev->biofeedbackEnabled ? "ENABLED" : "DISABLED");
                break;
                
            case 't':
                // Test PWM
                {
                    LOG_INFO("Manual PWM test triggered");
                    PWMConfig config;
                    config.frequency_hz_x100 = 72700;  // 727 Hz
                    config.duty_cycle = 50;
                    config.modulation_type = MODULATION_NONE;
                    config.intensity_level = 2048;
                    config.duration_ms = 5000;
                    
                    if (pwm_set_config(&config)) {
                        pwm_start();
                        Serial.println(F("PWM started: 727 Hz, 5s"));
                    } else {
                        Serial.println(F("PWM config failed!"));
                    }
                }
                break;
                
            case 'x':
                // Stop PWM
                pwm_stop();
                Serial.println(F("PWM stopped"));
                break;
                
            case 'l':
                // Flush logs
                Serial.println(F("\n=== Log History ==="));
                logging_flush_to_serial();
                break;
                
            case 'e':
                // Event stats
                Serial.println(F("\n=== Event Stats ==="));
                Serial.print(F("Events in queue: "));
                Serial.println(event_get_count());
                Serial.print(F("Dropped events: "));
                Serial.println(event_get_dropped_count());
                break;
                
            case 'd':
                // Device detection scan
                {
                    Serial.println(F("\n=== Device Detection Scan ==="));
                    EffectorType_t eff = detect_effector();
                    
                    Serial.print(F("Detected effector: "));
                    switch (eff) {
                        case EFFECTOR_HELMHOLTZ: Serial.println(F("Helmholtz Coil")); break;
                        case EFFECTOR_OTIC: Serial.println(F("Otic Applicator")); break;
                        case EFFECTOR_CONTACT: Serial.println(F("Contact Electrodes")); break;
                        case EFFECTOR_WRAP: Serial.println(F("Wrap Applicator")); break;
                        default: Serial.println(F("None")); break;
                    }
                    
                    BioSensorStatus_t bio_status;
                    BioStatus_t bio = detect_biofeedback_sensors(&bio_status);
                    
                    Serial.print(F("Biofeedback status: "));
                    switch (bio) {
                        case BIO_READY: Serial.println(F("READY - All sensors OK")); break;
                        case BIO_PARTIAL: Serial.println(F("PARTIAL - Some sensors missing")); break;
                        case BIO_GSR_MISSING: Serial.println(F("GSR sensor missing")); break;
                        case BIO_PPG_MISSING: Serial.println(F("PPG sensor missing")); break;
                        case BIO_TEMP_MISSING: Serial.println(F("Temp sensor missing")); break;
                        default: Serial.println(F("ERROR")); break;
                    }
                    
                    if (bio_status.gsrConnected) {
                        Serial.print(F("  GSR Quality: "));
                        Serial.println(bio_status.gsrQuality, 2);
                    }
                    if (bio_status.ppgConnected) {
                        Serial.print(F("  PPG Quality: "));
                        Serial.println(bio_status.ppgQuality, 2);
                    }
                    
                    DeviceSystemState_t* dev = get_device_system_state();
                    Serial.print(F("Detection count: "));
                    Serial.println(dev->detectionCount);
                    Serial.print(F("Error count: "));
                    Serial.println(dev->errorCount);
                }
                break;
                
            case 'h':
            default:
                Serial.println(F("\nCommands:"));
                Serial.println(F("  s - System status"));
                Serial.println(F("  t - Test PWM (727 Hz, 5s)"));
                Serial.println(F("  x - Stop PWM"));
                Serial.println(F("  l - Show log history"));
                Serial.println(F("  e - Event statistics"));
                Serial.println(F("  d - Detect devices"));
                Serial.println(F("  h - This help"));
                break;
        }
    }
    
    // ================================================================
    // BEZPIECZEŃSTWO - CHECK EMERGENCY
    // ================================================================
    
    if (!SAFE_IS_SAFE()) {
        // Krytyczny błąd bezpieczeństwa!
        pwm_stop();
        
        // Migaj ostrzegawczo
        static uint32_t last_blink = 0;
        if (now - last_blink >= 200) {
            last_blink = now;
            digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
        }
        
        LOG_ERROR_F("Safety fault! Code: %d", SAFE_GET_ERROR());
    }
    
    // Krótka przerwa dla stabilności (ale nie blokująca)
    // delayMicroseconds(100);  // Opcjonalne
}
