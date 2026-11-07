#include "logger.h"

#include <atomic>
#include <cstdio>
#include <string>
#include <vector>

int main() {
    Logger& logger = Logger::getInstance();
    logger.setConsoleOutput(false);
    logger.setLogLevel(LogLevel::DEBUG);
    logger.clearHistory();
    logger.clearCallbacks();

    std::atomic<int> reentrant_callback_count{0};
    logger.addCallback([&](const LogEntry&) {
        if (reentrant_callback_count.fetch_add(1) == 0) {
            logger.log(LogLevel::DEBUG, "nested callback log", __FILE__, __LINE__);
        }
    });

    const std::string path = "logger-smoke.log";
    logger.setLogFile(path); // Initialization and nested callback are both observed.
    logger.log(LogLevel::INFO, "logger smoke test", __FILE__, __LINE__);

    const bool reentrant_callbacks_ok = reentrant_callback_count.load() == 3;
    const bool reentrant_history_ok = logger.getHistory().size() == 3;

    // Exercise concurrent writers, callback delivery, and bounded history.
    logger.clearCallbacks();
    logger.clearHistory();
    std::atomic<int> concurrent_callback_count{0};
    logger.addCallback([&](const LogEntry&) {
        concurrent_callback_count.fetch_add(1);
    });

    constexpr int thread_count = 8;
    constexpr int logs_per_thread = 250;
    std::vector<std::thread> writers;
    writers.reserve(thread_count);
    for (int thread = 0; thread < thread_count; ++thread) {
        writers.emplace_back([thread]() {
            for (int item = 0; item < logs_per_thread; ++item) {
                Logger::getInstance().log(
                    LogLevel::DEBUG,
                    "concurrent log " + std::to_string(thread) + ":" + std::to_string(item),
                    __FILE__, __LINE__);
            }
        });
    }
    for (auto& writer : writers) writer.join();

    const auto snapshot = logger.getHistory();
    const bool concurrent_callbacks_ok =
        concurrent_callback_count.load() == thread_count * logs_per_thread;
    const bool bounded_history_ok = snapshot.size() == 1000;

    logger.clearHistory();
    const bool snapshot_is_independent = snapshot.size() == 1000 && logger.getHistory().empty();

    logger.clearCallbacks();
    logger.setLogFile(""); // Disable file logging; the failed-open diagnostic is intentional.
    std::remove(path.c_str());

    return reentrant_callbacks_ok && reentrant_history_ok &&
           concurrent_callbacks_ok && bounded_history_ok && snapshot_is_independent ? 0 : 1;
}
