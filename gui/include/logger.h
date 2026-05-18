/**
 * @file logger.h
 * @brief System logowania z poziomami: DEBUG, INFO, WARNING, ERROR, CRITICAL
 * 
 * Obsługuje:
 * - Różne poziomy logowania
 * - Znaczniki czasowe
 * - Wątki (thread-safe)
 * - Wyjścia: konsola, plik, callback
 * - Tryb verbose
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <functional>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <atomic>
#include <thread>

/**
 * @brief Poziomy logowania
 */
enum class LogLevel {
    DEBUG = 0,      // Szczegółowe informacje debugowe
    INFO = 1,       // Informacje ogólne
    WARNING = 2,    // Ostrzeżenia
    ERROR = 3,      // Błędy
    CRITICAL = 4    // Błędy krytyczne
};

/**
 * @brief Struktura pojedynczego wpisu logu
 */
struct LogEntry {
    LogLevel level;
    std::string message;
    std::string timestamp;
    std::string source_file;
    int source_line;
    std::thread::id thread_id;
    
    LogEntry(LogLevel lvl, const std::string& msg, const std::string& file, int line)
        : level(lvl), message(msg), source_file(file), source_line(line) {
        // Generuj znacznik czasowy
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        timestamp = oss.str();
        
        thread_id = std::this_thread::get_id();
    }
    
    std::string toString() const {
        const char* level_strs[] = {"DEBUG", "INFO", "WARN", "ERROR", "CRIT"};
        std::ostringstream oss;
        oss << "[" << timestamp << "] ";
        oss << "[" << std::setw(5) << level_strs[static_cast<int>(level)] << "] ";
        oss << "[Thread:" << thread_id << "] ";
        oss << message;
        
        if (level >= LogLevel::WARNING) {
            oss << " (" << source_file << ":" << source_line << ")";
        }
        
        return oss.str();
    }
};

/**
 * @brief Klasa Logger - singleton do zarządzania logowaniem
 */
class Logger {
public:
    using LogCallback = std::function<void(const LogEntry&)>;
    
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    /**
     * @brief Ustaw minimalny poziom logowania
     */
    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(log_mutex);
        min_level = level;
    }
    
    /**
     * @brief Włącz/wyłącz tryb verbose
     */
    void setVerbose(bool enable) {
        std::lock_guard<std::mutex> lock(log_mutex);
        verbose_mode = enable;
        if (enable) {
            min_level = LogLevel::DEBUG;
        }
    }
    
    bool isVerbose() const { return verbose_mode; }
    
    /**
     * @brief Włącz/wyłącz logowanie do pliku
     */
    void setLogFile(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (log_file.is_open()) {
            log_file.close();
        }
        log_file.open(filepath, std::ios::app);
        if (log_file.is_open()) {
            logToFile = true;
            log(LogLevel::INFO, "Logger initialized", __FILE__, __LINE__);
        } else {
            logToFile = false;
            log(LogLevel::ERROR, "Failed to open log file: " + filepath, __FILE__, __LINE__);
        }
    }
    
    /**
     * @brief Dodaj callback dla nowych wpisów logu
     */
    void addCallback(LogCallback callback) {
        std::lock_guard<std::mutex> lock(callback_mutex);
        callbacks.push_back(callback);
    }
    
    /**
     * @brief Wyczyść wszystkie callbacki
     */
    void clearCallbacks() {
        std::lock_guard<std::mutex> lock(callback_mutex);
        callbacks.clear();
    }
    
    /**
     * @brief Zaloguj wiadomość
     */
    void log(LogLevel level, const std::string& message, const std::string& file, int line) {
        if (level < min_level) return;
        
        LogEntry entry(level, message, file, line);
        
        std::lock_guard<std::mutex> lock(log_mutex);
        
        // Log do konsoli
        if (console_output) {
            std::ostream& out = (level >= LogLevel::ERROR) ? std::cerr : std::cout;
            out << entry.toString() << std::endl;
        }
        
        // Log do pliku
        if (logToFile && log_file.is_open()) {
            log_file << entry.toString() << std::endl;
            log_file.flush();
        }
        
        // Wywołaj callbacki
        {
            std::lock_guard<std::mutex> cb_lock(callback_mutex);
            for (auto& cb : callbacks) {
                try {
                    cb(entry);
                } catch (const std::exception& e) {
                    std::cerr << "[Logger] Callback exception: " << e.what() << std::endl;
                }
            }
        }
        
        // Zapisz do historii
        log_history.push_back(entry);
        if (log_history.size() > max_history_size) {
            log_history.erase(log_history.begin());
        }
    }
    
    /**
     * @brief Pobierz historię logów
     */
    const std::vector<LogEntry>& getHistory() const {
        return log_history;
    }
    
    /**
     * @brief Wyczyść historię logów
     */
    void clearHistory() {
        std::lock_guard<std::mutex> lock(log_mutex);
        log_history.clear();
    }
    
    /**
     * @brief Włącz/wyłącz wyjście na konsolę
     */
    void setConsoleOutput(bool enable) {
        std::lock_guard<std::mutex> lock(log_mutex);
        console_output = enable;
    }
    
    /**
     * @brief Get log level as string
     */
    static std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
private:
    Logger() : min_level(LogLevel::INFO), verbose_mode(false), 
               logToFile(false), console_output(true), max_history_size(1000) {}
    
    ~Logger() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    LogLevel min_level;
    bool verbose_mode;
    bool logToFile;
    bool console_output;
    size_t max_history_size;
    
    std::ofstream log_file;
    std::mutex log_mutex;
    std::mutex callback_mutex;
    std::vector<LogCallback> callbacks;
    std::vector<LogEntry> log_history;
};

// Makra ułatwiające logowanie
#define LOG_DEBUG(msg) Logger::getInstance().log(LogLevel::DEBUG, msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Logger::getInstance().log(LogLevel::INFO, msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::getInstance().log(LogLevel::WARNING, msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::getInstance().log(LogLevel::ERROR, msg, __FILE__, __LINE__)
#define LOG_CRITICAL(msg) Logger::getInstance().log(LogLevel::CRITICAL, msg, __FILE__, __LINE__)

// Log z warunkiem verbose
#define LOG_VERBOSE(msg) \
    do { \
        if (Logger::getInstance().isVerbose()) { \
            Logger::getInstance().log(LogLevel::DEBUG, msg, __FILE__, __LINE__); \
        } \
    } while(0)

#endif // LOGGER_H
