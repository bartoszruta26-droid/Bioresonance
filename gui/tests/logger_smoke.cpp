#include "logger.h"

#include <atomic>
#include <cstdio>
#include <string>

int main() {
    Logger& logger = Logger::getInstance();
    logger.setConsoleOutput(false);
    logger.setLogLevel(LogLevel::DEBUG);
    logger.clearHistory();
    logger.clearCallbacks();

    std::atomic<int> callback_count{0};
    logger.addCallback([&](const LogEntry&) {
        if (callback_count.fetch_add(1) == 0) {
            logger.log(LogLevel::DEBUG, "nested callback log", __FILE__, __LINE__);
        }
    });

    const std::string path = "logger-smoke.log";
    logger.setLogFile(path); // One callback plus its nested callback.
    logger.log(LogLevel::INFO, "logger smoke test", __FILE__, __LINE__);

    const auto history = logger.getHistory();
    const bool callbacks_ok = callback_count.load() == 3;
    const bool history_ok = history.size() == 3;
    logger.clearCallbacks();
    logger.setLogFile(""); // Disable file logging via failed open of empty path.
    std::remove(path.c_str());
    return callbacks_ok && history_ok ? 0 : 1;
}
