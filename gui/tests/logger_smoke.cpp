#include "logger.h"

#include <atomic>
#include <cstdio>
#include <string>

int main() {
    Logger& logger = Logger::getInstance();
    logger.setConsoleOutput(false);
    logger.setLogLevel(LogLevel::DEBUG);

    std::atomic<int> callback_count{0};
    logger.addCallback([&](const LogEntry&) {
        if (callback_count.fetch_add(1) == 0) {
            // Reentrant logging must not deadlock on the logger's internal locks.
            logger.log(LogLevel::DEBUG, "nested callback log", __FILE__, __LINE__);
        }
    });

    const std::string path = "logger-smoke.log";
    logger.setLogFile(path);
    logger.log(LogLevel::INFO, "logger smoke test", __FILE__, __LINE__);

    const bool callbacks_ok = callback_count.load() == 3;
    std::remove(path.c_str());
    return callbacks_ok ? 0 : 1;
}
