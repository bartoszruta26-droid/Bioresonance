/**
 * @file event_system.cpp
 * @brief System obsługi zdarzeń (Event Handling) - Implementacja
 * @version 4.0 (Modular Architecture with Pseudo-threading)
 */

#include "event_system.h"
#include "logging_system.h"

// ============================================================================
// ZMIENNE PRYWATNE
// ============================================================================

static EventQueue event_queue;

// ============================================================================
// IMPLEMENTACJA FUNKCJI PUBLICZNYCH
// ============================================================================

/**
 * @brief Inicjalizuje system zdarzeń
 */
void event_system_init() {
    cli();
    event_queue.head = 0;
    event_queue.tail = 0;
    event_queue.count = 0;
    event_queue.dropped_count = 0;
    sei();
    
    EVENT_INFO(EVENT_SYSTEM_RESET, "Event system initialized");
}

/**
 * @brief Pętla systemu zdarzeń (pseudo-wątek)
 * Przetwarza zdarzenia z kolejki
 */
void event_system_loop() {
    // Przetwarzaj zdarzenia z kolejki
    SystemEvent event;
    
    while (event_pop(&event)) {
        // Tutaj można dodać dodatkową logikę przetwarzania
        // Na razie tylko logujemy wszystkie zdarzenia
        
        // Specjalna obsługa krytycznych zdarzeń
        if (event.severity == EVENT_SEVERITY_CRITICAL) {
            // Natychmiastowe logowanie krytycznych zdarzeń
            LOG_EVENT(LOG_LEVEL_FATAL, event.event_code, event.description);
            
            // Można dodać dodatkowe akcje dla zdarzeń krytycznych
            // np. natychmiastowe wysłanie alertu przez sieć
        }
    }
}

/**
 * @brief Dodaje zdarzenie do kolejki
 * @param event_code Kod zdarzenia
 * @param severity Poziom ważności
 * @param data Dodatkowe dane
 * @param description Opis zdarzenia
 * @return true jeśli dodano sukcesem
 */
bool event_push(uint16_t event_code, uint8_t severity, uint32_t data, const char* description) {
    cli();
    
    // Sprawdź przepełnienie
    if (event_queue.count >= EVENT_QUEUE_SIZE) {
        event_queue.dropped_count++;
        event_queue.tail = (event_queue.tail + 1) % EVENT_QUEUE_SIZE;
        event_queue.count--;
        
        // Loguj przepełnienie (ale ostrożnie żeby nie rekurencja)
        if (severity != EVENT_SEVERITY_CRITICAL) {
            sei();
            LOG_WARN("Event queue overflow - dropped event");
            cli();
        }
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
    
    sei();
    
    // Dla zdarzeń krytycznych natychmiastowe logowanie
    if (severity == EVENT_SEVERITY_CRITICAL || severity == EVENT_SEVERITY_ERROR) {
        char log_msg[64];
        snprintf_P(log_msg, sizeof(log_msg), PSTR("Event 0x%04X [%s]"), event_code, description);
        log_add_entry(severity == EVENT_SEVERITY_CRITICAL ? LOG_LEVEL_FATAL : LOG_LEVEL_ERROR, 
                      event_code, log_msg);
    }
    
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
 * @brief Podgląda następne zdarzenie bez usuwania
 * @param event Wskaźnik do struktury zdarzenia
 * @return true jeśli dostępne zdarzenie
 */
bool event_peek(SystemEvent* event) {
    if (!event) return false;
    
    cli();
    
    if (event_queue.count == 0) {
        sei();
        return false;
    }
    
    // Skopiuj pierwsze zdarzenie
    memcpy(event, &event_queue.events[event_queue.tail], sizeof(SystemEvent));
    
    sei();
    return true;
}

/**
 * @brief Pobiera liczbę zdarzeń w kolejce
 * @return Liczba zdarzeń
 */
uint16_t event_get_count() {
    uint16_t count;
    cli();
    count = event_queue.count;
    sei();
    return count;
}

/**
 * @brief Pobiera liczbę utraconych zdarzeń
 * @return Liczba odrzuconych zdarzeń
 */
uint16_t event_get_dropped_count() {
    uint16_t count;
    cli();
    count = event_queue.dropped_count;
    sei();
    return count;
}

/**
 * @brief Czyści całą kolejkę zdarzeń
 */
void event_clear_queue() {
    cli();
    event_queue.head = 0;
    event_queue.tail = 0;
    event_queue.count = 0;
    sei();
}
