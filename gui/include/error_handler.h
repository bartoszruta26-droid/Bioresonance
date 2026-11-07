/**
 * @file error_handler.h
 * @brief System obsługi błędów z kodami, kategoriami i mechanizmami recovery
 * 
 * Obsługuje:
 * - Kody błędów z kategoriami
 * - Stack trace (gdzie dostępne)
 * - Callbacki dla błędów
 * - Automatyczne retry
 * - Wyjątki C++
 */

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include <cstddef>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <memory>
#include <map>
#include <sstream>
#include "logger.h"

/**
 * @brief Kategorie błędów
 */
enum class ErrorCategory {
    NONE = 0,
    NETWORK,        // Błędy sieciowe
    FILE_IO,        // Błędy plików
    MEMORY,         // Błędy pamięci
    HARDWARE,       // Błędy sprzętowe
    CONFIG,         // Błędy konfiguracji
    PROTOCOL,       // Błędy protokołu
    USER,           // Błędy użytkownika
    SYSTEM,         // Błędy systemowe
    UNKNOWN         // Nieznana kategoria
};

/**
 * @brief Kody błędów
 */
enum class ErrorCode {
    SUCCESS = 0,
    
    // Network errors (100-199)
    ERR_NETWORK_TIMEOUT = 100,
    ERR_NETWORK_DISCONNECTED = 101,
    ERR_NETWORK_UNREACHABLE = 102,
    ERR_NETWORK_DNS_FAILED = 103,
    ERR_NETWORK_SOCKET_FAILED = 104,
    ERR_NETWORK_SEND_FAILED = 105,
    ERR_NETWORK_RECV_FAILED = 106,
    
    // File I/O errors (200-299)
    ERR_FILE_NOT_FOUND = 200,
    ERR_FILE_PERMISSION_DENIED = 201,
    ERR_FILE_READ_FAILED = 202,
    ERR_FILE_WRITE_FAILED = 203,
    ERR_FILE_CORRUPTED = 204,
    
    // Memory errors (300-399)
    ERR_MEMORY_ALLOCATION = 300,
    ERR_MEMORY_OVERFLOW = 301,
    
    // Hardware errors (400-499)
    ERR_HARDWARE_NOT_FOUND = 400,
    ERR_HARDWARE_COMMUNICATION = 401,
    ERR_HARDWARE_OVERHEAT = 402,
    ERR_HARDWARE_POWER = 403,
    
    // Config errors (500-599)
    ERR_CONFIG_INVALID = 500,
    ERR_CONFIG_MISSING = 501,
    ERR_CONFIG_PARSE_FAILED = 502,
    
    // Protocol errors (600-699)
    ERR_PROTOCOL_INVALID_FORMAT = 600,
    ERR_PROTOCOL_CHECKSUM = 601,
    ERR_PROTOCOL_TIMEOUT = 602,
    ERR_PROTOCOL_VERSION = 603,
    
    // User errors (700-799)
    ERR_USER_INPUT_INVALID = 700,
    ERR_USER_CANCELLED = 701,
    
    // System errors (800-899)
    ERR_SYSTEM_CALL_FAILED = 800,
    ERR_SYSTEM_RESOURCE_UNAVAILABLE = 801,
    ERR_SYSTEM_PERMISSION = 802,
    
    // Unknown
    ERR_UNKNOWN = 999
};

/**
 * @brief Struktura informacji o błędzie
 */
struct ErrorInfo {
    ErrorCode code;
    ErrorCategory category;
    std::string message;
    std::string details;
    std::string source_file;
    int source_line;
    std::string function;
    std::chrono::system_clock::time_point timestamp;
    int retry_count;
    bool recoverable;
    
    ErrorInfo() : code(ErrorCode::SUCCESS), category(ErrorCategory::NONE),
                  source_line(0), retry_count(0), recoverable(false) {
        timestamp = std::chrono::system_clock::now();
    }
    
    ErrorInfo(ErrorCode c, ErrorCategory cat, const std::string& msg,
              const std::string& file, int line, const std::string& func = "")
        : code(c), category(cat), message(msg), source_file(file),
          source_line(line), function(func), retry_count(0), recoverable(true) {
        timestamp = std::chrono::system_clock::now();
    }
    
    std::string toString() const {
        std::ostringstream oss;
        oss << "Error [" << static_cast<int>(code) << "] ";
        oss << "(" << ErrorInfo::categoryToString(category) << "): ";
        oss << message;
        
        if (!details.empty()) {
            oss << " - " << details;
        }
        
        if (!source_file.empty()) {
            oss << " at " << source_file << ":" << source_line;
        }
        
        return oss.str();
    }
    
    static std::string categoryToString(ErrorCategory cat) {
        switch (cat) {
            case ErrorCategory::NETWORK: return "NETWORK";
            case ErrorCategory::FILE_IO: return "FILE_IO";
            case ErrorCategory::MEMORY: return "MEMORY";
            case ErrorCategory::HARDWARE: return "HARDWARE";
            case ErrorCategory::CONFIG: return "CONFIG";
            case ErrorCategory::PROTOCOL: return "PROTOCOL";
            case ErrorCategory::USER: return "USER";
            case ErrorCategory::SYSTEM: return "SYSTEM";
            default: return "UNKNOWN";
        }
    }
    
    static std::string codeToString(ErrorCode code) {
        switch (code) {
            case ErrorCode::SUCCESS: return "SUCCESS";
            case ErrorCode::ERR_NETWORK_TIMEOUT:
            case ErrorCode::ERR_PROTOCOL_TIMEOUT: return "TRANSPORT_TIMEOUT";
            case ErrorCode::ERR_NETWORK_DISCONNECTED: return "CONNECTION_CLOSED";
            case ErrorCode::ERR_NETWORK_UNREACHABLE:
            case ErrorCode::ERR_NETWORK_DNS_FAILED: return "TRANSPORT_UNAVAILABLE";
            case ErrorCode::ERR_NETWORK_SEND_FAILED:
            case ErrorCode::ERR_NETWORK_RECV_FAILED: return "DEVICE_COMMUNICATION_FAILED";
            case ErrorCode::ERR_NETWORK_SOCKET_FAILED: return "INTERNAL_ERROR";
            case ErrorCode::ERR_FILE_NOT_FOUND:
            case ErrorCode::ERR_FILE_PERMISSION_DENIED:
            case ErrorCode::ERR_FILE_READ_FAILED: return "FILE_READ_FAILED";
            case ErrorCode::ERR_FILE_WRITE_FAILED: return "FILE_WRITE_FAILED";
            case ErrorCode::ERR_FILE_CORRUPTED: return "FILE_INVALID_FORMAT";
            case ErrorCode::ERR_PROTOCOL_INVALID_FORMAT:
            case ErrorCode::ERR_PROTOCOL_VERSION: return "PROTOCOL_INVALID_RESPONSE";
            case ErrorCode::ERR_PROTOCOL_CHECKSUM: return "PROTOCOL_CRC_MISMATCH";
            case ErrorCode::ERR_USER_INPUT_INVALID: return "INVALID_REQUEST";
            case ErrorCode::ERR_HARDWARE_NOT_FOUND: return "DEVICE_UNAVAILABLE";
            case ErrorCode::ERR_UNKNOWN: return "INTERNAL_ERROR";
            default: return "INTERNAL_ERROR";
        }
    }
};

/**
 * @brief Wyjątek z informacjami o błędzie
 */
class Exception : public std::runtime_error {
public:
    Exception(const ErrorInfo& info) 
        : std::runtime_error(info.message), error_info(info) {}
    
    Exception(ErrorCode code, ErrorCategory cat, const std::string& msg,
              const std::string& file = __FILE__, int line = __LINE__)
        : std::runtime_error(msg), error_info(code, cat, msg, file, line) {}
    
    const ErrorInfo& getErrorInfo() const { return error_info; }
    
private:
    ErrorInfo error_info;
};

// Makra ułatwiające zgłaszanie błędów
#define THROW_ERROR(code, cat, msg) \
    throw Exception(code, cat, msg, __FILE__, __LINE__)

#define THROW_IF(condition, code, cat, msg) \
    do { if (condition) { THROW_ERROR(code, cat, msg); } } while(0)

#define RETURN_ON_ERROR(expr) \
    do { \
        auto _result = (expr); \
        if (_result != ErrorCode::SUCCESS) { return _result; } \
    } while(0)

/**
 * @brief Klasa ErrorHandler - singleton do zarządzania błędami
 */
class ErrorHandler {
public:
    using ErrorCallback = std::function<void(const ErrorInfo&)>;
    using RecoveryCallback = std::function<bool(const ErrorInfo&)>;
    
    static ErrorHandler& getInstance() {
        static ErrorHandler instance;
        return instance;
    }
    
    /**
     * @brief Zarejestruj callback dla błędów
     */
    void registerCallback(ErrorCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_.push_back(callback);
    }
    
    /**
     * @brief Zarejestruj handler recovery dla kategorii błędów
     */
    void registerRecoveryHandler(ErrorCategory category, RecoveryCallback handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        recovery_handlers_[category] = handler;
    }
    
    /**
     * @brief Zgłoś błąd
     */
    void reportError(const ErrorInfo& error) {
        std::vector<ErrorCallback> callbacks_snapshot;
        RecoveryCallback recovery_callback;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            callbacks_snapshot = callbacks_;
            const auto recovery = recovery_handlers_.find(error.category);
            if (error.recoverable && recovery != recovery_handlers_.end()) {
                recovery_callback = recovery->second;
            }

            error_history_.push_back(error);
            if (error_history_.size() > max_history_size_) {
                error_history_.erase(error_history_.begin(),
                    error_history_.begin() + static_cast<std::ptrdiff_t>(
                        error_history_.size() - max_history_size_));
            }
            if (error.category != ErrorCategory::USER) {
                ++error_count_;
            }
        }

        // Run logging and user callbacks outside mutex_: callbacks may report
        // another error or register handlers without recursively deadlocking.
        LogLevel log_level = LogLevel::ERROR;
        if (error.category == ErrorCategory::HARDWARE ||
            error.category == ErrorCategory::SYSTEM) {
            log_level = LogLevel::CRITICAL;
        }
        Logger::getInstance().log(log_level, error.toString(),
                                  error.source_file, error.source_line);

        for (const auto& callback : callbacks_snapshot) {
            try {
                if (callback) callback(error);
            } catch (const std::exception& e) {
                Logger::getInstance().log(LogLevel::ERROR,
                    std::string("Error callback exception: ") + e.what(),
                    __FILE__, __LINE__);
            } catch (...) {
                Logger::getInstance().log(LogLevel::ERROR,
                    "Error callback raised a non-standard exception",
                    __FILE__, __LINE__);
            }
        }

        if (recovery_callback) {
            try {
                if (recovery_callback(error)) {
                    Logger::getInstance().log(LogLevel::INFO,
                        "Successfully recovered from error: " + error.message,
                        __FILE__, __LINE__);
                }
            } catch (const std::exception& e) {
                Logger::getInstance().log(LogLevel::ERROR,
                    std::string("Recovery handler exception: ") + e.what(),
                    __FILE__, __LINE__);
            } catch (...) {
                Logger::getInstance().log(LogLevel::ERROR,
                    "Recovery handler raised a non-standard exception",
                    __FILE__, __LINE__);
            }
        }
    }

    /**
     * @brief Pobierz historię błędów
     */
    std::vector<ErrorInfo> getErrorHistory() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return error_history_;
    }
    
    /**
     * @brief Wyczyść historię błędów
     */
    void clearErrorHistory() {
        std::lock_guard<std::mutex> lock(mutex_);
        error_history_.clear();
    }
    
    /**
     * @brief Pobierz liczbę błędów
     */
    int getErrorCount() const {
        return error_count_.load();
    }
    
    /**
     * @brief Resetuj licznik błędów
     */
    void resetErrorCount() {
        error_count_ = 0;
    }
    
    /**
     * @brief Sprawdź czy wystąpił błąd krytyczny
     */
    bool hasCriticalError() const {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& err : error_history_) {
            if (err.category == ErrorCategory::HARDWARE ||
                err.category == ErrorCategory::SYSTEM ||
                err.category == ErrorCategory::MEMORY) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * @brief Ustaw maksymalny rozmiar historii
     */
    void setMaxHistorySize(size_t size) {
        std::lock_guard<std::mutex> lock(mutex_);
        max_history_size_ = size;
        if (error_history_.size() > max_history_size_) {
            error_history_.erase(error_history_.begin(),
                error_history_.begin() + static_cast<std::ptrdiff_t>(
                    error_history_.size() - max_history_size_));
        }
    }

private:
    ErrorHandler() : error_count_(0), max_history_size_(100) {}
    ~ErrorHandler() = default;
    
    ErrorHandler(const ErrorHandler&) = delete;
    ErrorHandler& operator=(const ErrorHandler&) = delete;
    
    mutable std::mutex mutex_;
    std::vector<ErrorCallback> callbacks_;
    std::map<ErrorCategory, RecoveryCallback> recovery_handlers_;
    std::vector<ErrorInfo> error_history_;
    std::atomic<int> error_count_;
    size_t max_history_size_;
};

// Makra ułatwiające raportowanie błędów
#define REPORT_ERROR(code, cat, msg) \
    ErrorHandler::getInstance().reportError(ErrorInfo(code, cat, msg, __FILE__, __LINE__))

#define REPORT_ERROR_DETAILS(code, cat, msg, details) \
    do { \
        ErrorInfo err(code, cat, msg, __FILE__, __LINE__); \
        err.details = details; \
        ErrorHandler::getInstance().reportError(err); \
    } while(0)

#endif // ERROR_HANDLER_H
