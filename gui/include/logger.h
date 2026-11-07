/**
 * @file logger.h
 * @brief Thread-safe logging with bounded history and reentrant callbacks.
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <atomic>
#include <chrono>
#include <cstddef>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// Windows SDK headers may define ERROR/CRITICAL as macros.
#ifdef ERROR
#undef ERROR
#endif
#ifdef CRITICAL
#undef CRITICAL
#endif

enum class LogLevel { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3, CRITICAL = 4 };

struct LogEntry {
    LogLevel level;
    std::string message;
    std::string timestamp;
    std::string source_file;
    int source_line;
    std::thread::id thread_id;

    LogEntry(LogLevel lvl, const std::string& msg, const std::string& file, int line)
        : level(lvl), message(msg), source_file(file), source_line(line) {
        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(now);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        std::tm time_info{};
#ifdef _WIN32
        localtime_s(&time_info, &time);
#else
        localtime_r(&time, &time_info);
#endif
        std::ostringstream out;
        out << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << ms.count();
        timestamp = out.str();
        thread_id = std::this_thread::get_id();
    }

    std::string toString() const {
        static const char* levels[] = {"DEBUG", "INFO", "WARN", "ERROR", "CRIT"};
        const auto index = static_cast<std::size_t>(level);
        std::ostringstream out;
        out << '[' << timestamp << "] ["
            << ((index < sizeof(levels) / sizeof(levels[0])) ? levels[index] : "UNKNOWN")
            << "] [Thread:" << thread_id << "] " << message;
        if (level >= LogLevel::WARNING) out << " (" << source_file << ':' << source_line << ')';
        return out.str();
    }
};

class Logger {
public:
    using LogCallback = std::function<void(const LogEntry&)>;

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(log_mutex);
        min_level = level;
    }

    void setVerbose(bool enable) {
        std::lock_guard<std::mutex> lock(log_mutex);
        verbose_mode = enable;
        if (enable) min_level = LogLevel::DEBUG;
    }

    bool isVerbose() const { return verbose_mode.load(); }

    void setLogFile(const std::string& filepath) {
        bool opened = false;
        {
            std::lock_guard<std::mutex> lock(log_mutex);
            if (log_file.is_open()) log_file.close();
            log_file.clear();
            log_file.open(filepath, std::ios::app);
            opened = log_file.is_open();
            logToFile = opened;
        }
        // Logging after releasing log_mutex avoids recursive locking.
        log(opened ? LogLevel::INFO : LogLevel::ERROR,
            opened ? "Logger initialized" : "Failed to open log file: " + filepath,
            __FILE__, __LINE__);
    }

    void addCallback(LogCallback callback) {
        if (!callback) return;
        std::lock_guard<std::mutex> lock(callback_mutex);
        callbacks.push_back(std::move(callback));
    }

    void clearCallbacks() {
        std::lock_guard<std::mutex> lock(callback_mutex);
        callbacks.clear();
    }

    void log(LogLevel level, const std::string& message, const std::string& file, int line) {
        LogEntry entry(level, message, file, line);
        {
            std::lock_guard<std::mutex> lock(log_mutex);
            if (level < min_level) return;
            if (console_output) {
                std::ostream& output = level >= LogLevel::ERROR ? std::cerr : std::cout;
                output << entry.toString() << std::endl;
            }
            if (logToFile && log_file.is_open()) {
                log_file << entry.toString() << std::endl;
                log_file.flush();
            }
            log_history.push_back(entry);
            if (log_history.size() > max_history_size) {
                log_history.erase(log_history.begin(),
                    log_history.begin() + static_cast<std::ptrdiff_t>(log_history.size() - max_history_size));
            }
        }

        std::vector<LogCallback> callbacks_snapshot;
        {
            std::lock_guard<std::mutex> lock(callback_mutex);
            callbacks_snapshot = callbacks;
        }
        for (const auto& callback : callbacks_snapshot) {
            try { callback(entry); }
            catch (const std::exception& error) {
                std::cerr << "[Logger] Callback exception: " << error.what() << std::endl;
            }
            catch (...) {
                std::cerr << "[Logger] Callback exception: unknown exception" << std::endl;
            }
        }
    }

    // Returns a snapshot; callers never access the mutable internal vector.
    std::vector<LogEntry> getHistory() const {
        std::lock_guard<std::mutex> lock(log_mutex);
        return log_history;
    }

    void clearHistory() {
        std::lock_guard<std::mutex> lock(log_mutex);
        log_history.clear();
    }

    void setConsoleOutput(bool enable) {
        std::lock_guard<std::mutex> lock(log_mutex);
        console_output = enable;
    }

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
    Logger() : min_level(LogLevel::INFO), verbose_mode(false), logToFile(false),
               console_output(true), max_history_size(1000) {}
    ~Logger() {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (log_file.is_open()) log_file.close();
    }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel min_level;
    std::atomic<bool> verbose_mode;
    bool logToFile;
    bool console_output;
    std::size_t max_history_size;
    std::ofstream log_file;
    mutable std::mutex log_mutex;
    std::mutex callback_mutex;
    std::vector<LogCallback> callbacks;
    std::vector<LogEntry> log_history;
};

#define LOG_DEBUG(msg) Logger::getInstance().log(LogLevel::DEBUG, msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Logger::getInstance().log(LogLevel::INFO, msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::getInstance().log(LogLevel::WARNING, msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::getInstance().log(LogLevel::ERROR, msg, __FILE__, __LINE__)
#define LOG_CRITICAL(msg) Logger::getInstance().log(LogLevel::CRITICAL, msg, __FILE__, __LINE__)
#define LOG_VERBOSE(msg) do { if (Logger::getInstance().isVerbose()) Logger::getInstance().log(LogLevel::DEBUG, msg, __FILE__, __LINE__); } while (0)

#endif // LOGGER_H
