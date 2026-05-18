/**
 * @file event_system.h
 * @brief System zdarzeń (event-driven) z subskrypcją i publikacją
 * 
 * Obsługuje:
 * - Różne typy zdarzeń
 * - Subskrypcja/odsubskrypcja handlerów
 * - Asynchroniczne przetwarzanie
 * - Priorytety zdarzeń
 * - Filtrowanie zdarzeń
 */

#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <memory>
#include <chrono>
#include "logger.h"

/**
 * @brief Typy zdarzeń w aplikacji
 */
enum class EventType {
    NONE = 0,
    
    // Application events (1-99)
    APP_START = 1,
    APP_STOP = 2,
    APP_PAUSE = 3,
    APP_RESUME = 4,
    
    // Connection events (100-199)
    CONNECT_REQUEST = 100,
    CONNECT_SUCCESS = 101,
    CONNECT_FAILED = 102,
    DISCONNECT = 103,
    CONNECTION_LOST = 104,
    
    // Probe events (200-299)
    PROBE_SELECTED = 200,
    PROBE_ENABLED = 201,
    PROBE_DISABLED = 202,
    PROBE_CONFIG_CHANGED = 203,
    PROBE_ERROR = 204,
    
    // Therapy events (300-399)
    THERAPY_START = 300,
    THERAPY_STOP = 301,
    THERAPY_PROGRESS = 302,
    THERAPY_COMPLETE = 303,
    THERAPY_ERROR = 304,
    
    // Frequency events (400-499)
    FREQUENCY_SELECTED = 400,
    FREQUENCY_CHANGED = 401,
    FREQUENCY_SEARCH = 402,
    
    // UI events (500-599)
    UI_TAB_CHANGED = 500,
    UI_WINDOW_RESIZED = 501,
    UI_THEME_CHANGED = 502,
    UI_DIALOG_OPENED = 503,
    UI_DIALOG_CLOSED = 504,
    
    // Network events (600-699)
    NETWORK_DATA_RECEIVED = 600,
    NETWORK_DATA_SENT = 601,
    NETWORK_TIMEOUT = 602,
    NETWORK_ERROR = 603,
    
    // Error events (700-799)
    ERROR_OCCURRED = 700,
    ERROR_RECOVERED = 701,
    
    // Custom events (1000+)
    CUSTOM_EVENT = 1000
};

/**
 * @brief Priorytety zdarzeń
 */
enum class EventPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

/**
 * @brief Struktura zdarzenia
 */
struct Event {
    EventType type;
    EventPriority priority;
    std::string source;
    std::string data;
    void* payload;
    std::chrono::system_clock::time_point timestamp;
    int sequence_number;
    bool handled;
    
    Event() : type(EventType::NONE), priority(EventPriority::NORMAL),
              payload(nullptr), sequence_number(0), handled(false) {
        timestamp = std::chrono::system_clock::now();
    }
    
    Event(EventType t, const std::string& src = "", const std::string& d = "",
          void* p = nullptr, EventPriority prio = EventPriority::NORMAL)
        : type(t), priority(prio), source(src), data(d), payload(p), handled(false) {
        timestamp = std::chrono::system_clock::now();
        static std::atomic<int> seq_counter{0};
        sequence_number = seq_counter++;
    }
    
    ~Event() {
        // Uwaga: payload nie jest usuwany przez Event
        // Właściciel payload musi zadbać o jego cleanup
    }
    
    std::string toString() const {
        return "Event[" + std::to_string(static_cast<int>(type)) + "] " +
               "from " + source + ": " + data;
    }
};

/**
 * @brief Handler zdarzenia
 */
using EventHandler = std::function<void(const Event&)>;

/**
 * @brief Informacja o subskrypcji
 */
struct Subscription {
    int id;
    EventType event_type;
    EventHandler handler;
    EventPriority min_priority;
    bool active;
    
    Subscription(int i, EventType t, EventHandler h, EventPriority p = EventPriority::LOW)
        : id(i), event_type(t), handler(h), min_priority(p), active(true) {}
};

/**
 * @brief Klasa EventSystem - system zarządzania zdarzeniami
 */
class EventSystem {
public:
    static EventSystem& getInstance() {
        static EventSystem instance;
        return instance;
    }
    
    /**
     * @brief Subskrybuj handler dla typu zdarzenia
     * @return ID subskrypcji
     */
    int subscribe(EventType event_type, EventHandler handler, 
                  EventPriority min_priority = EventPriority::LOW) {
        std::lock_guard<std::mutex> lock(mutex_);
        int id = next_subscription_id_++;
        subscriptions_.emplace_back(id, event_type, handler, min_priority);
        LOG_DEBUG("Subscribed to event " + std::to_string(static_cast<int>(event_type)) + 
                  " with ID " + std::to_string(id));
        return id;
    }
    
    /**
     * @brief Odsubskrybuj handler
     */
    void unsubscribe(int subscription_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& sub : subscriptions_) {
            if (sub.id == subscription_id) {
                sub.active = false;
                LOG_DEBUG("Unsubscribed ID " + std::to_string(subscription_id));
                return;
            }
        }
    }
    
    /**
     * @brief Odsubskrybuj wszystkie handlery dla typu zdarzenia
     */
    void unsubscribeAll(EventType event_type) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& sub : subscriptions_) {
            if (sub.event_type == event_type) {
                sub.active = false;
            }
        }
    }
    
    /**
     * @brief Opublikuj zdarzenie (synchronicznie)
     */
    void publish(const Event& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        LOG_VERBOSE("Publishing event: " + event.toString());
        
        // Sortuj subskrypcje według priorytetu
        std::vector<Subscription*> matching_subs;
        for (auto& sub : subscriptions_) {
            if (sub.active && 
                (sub.event_type == event.type || sub.event_type == EventType::NONE) &&
                sub.min_priority <= event.priority) {
                matching_subs.push_back(&sub);
            }
        }
        
        // Wywołaj handlery
        for (auto* sub : matching_subs) {
            try {
                sub->handler(event);
            } catch (const std::exception& e) {
                LOG_ERROR("Event handler exception: " + std::string(e.what()));
            }
        }
    }
    
    /**
     * @brief Opublikuj zdarzenie (asynchronicznie - queue)
     * Thread-safe version with proper memory management
     */
    void publishAsync(const Event& event) {
        // Create a copy of event to avoid dangling references
        Event event_copy = event;
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            event_queue_.push(event_copy);
        }
        
        // Start processing thread if not already running
        if (!processing_thread_.joinable() || !processing_active_) {
            startProcessingThread();
        }
    }
    
    /**
     * @brief Wyczyść kolejkę zdarzeń
     */
    void clearQueue() {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (!event_queue_.empty()) {
            event_queue_.pop();
        }
    }
    
    /**
     * @brief Rozmiar kolejki
     */
    size_t queueSize() const {
        return event_queue_.size();
    }
    
    /**
     * @brief Start processing thread
     */
    void startProcessingThread() {
        if (!processing_thread_.joinable() || !processing_active_) {
            processing_active_ = true;
            processing_thread_ = std::thread([this]() {
                processEventQueue();
            });
        }
    }
    
    /**
     * @brief Stop processing thread
     */
    void stopProcessingThread() {
        processing_active_ = false;
        if (processing_thread_.joinable()) {
            processing_thread_.join();
        }
    }
    
    /**
     * @brief Get event type as string
     */
    static std::string eventTypeToString(EventType type) {
        switch (type) {
            case EventType::APP_START: return "APP_START";
            case EventType::APP_STOP: return "APP_STOP";
            case EventType::CONNECT_SUCCESS: return "CONNECT_SUCCESS";
            case EventType::CONNECT_FAILED: return "CONNECT_FAILED";
            case EventType::DISCONNECT: return "DISCONNECT";
            case EventType::PROBE_SELECTED: return "PROBE_SELECTED";
            case EventType::THERAPY_START: return "THERAPY_START";
            case EventType::THERAPY_STOP: return "THERAPY_STOP";
            case EventType::FREQUENCY_SELECTED: return "FREQUENCY_SELECTED";
            case EventType::ERROR_OCCURRED: return "ERROR_OCCURRED";
            default: return "EVENT_" + std::to_string(static_cast<int>(type));
        }
    }

private:
    EventSystem() : next_subscription_id_(0), processing_active_(false) {}
    
    ~EventSystem() {
        stopProcessingThread();
    }
    
    EventSystem(const EventSystem&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;
    
    void processEventQueue() {
        while (processing_active_) {
            Event event;
            bool has_event = false;
            
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                if (!event_queue_.empty()) {
                    event = event_queue_.front();
                    event_queue_.pop();
                    has_event = true;
                }
            }
            
            if (!has_event) {
                // Wait a bit before checking again
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            
            // Process based on priority
            publish(event);  // Call publish which has its own lock
        }
    }
    
    std::mutex mutex_;
    std::vector<Subscription> subscriptions_;
    int next_subscription_id_;
    
    std::mutex queue_mutex_;
    std::queue<Event> event_queue_;
    std::thread processing_thread_;
    std::atomic<bool> processing_active_;
};

// Makra ułatwiające publikację zdarzeń
#define PUBLISH_EVENT(type, src, data) \
    EventSystem::getInstance().publish(Event(type, src, data))

#define PUBLISH_EVENT_ASYNC(type, src, data) \
    EventSystem::getInstance().publishAsync(Event(type, src, data))

#define PUBLISH_EVENT_WITH_PAYLOAD(type, src, data, payload) \
    EventSystem::getInstance().publish(Event(type, src, data, payload))

#endif // EVENT_SYSTEM_H
