#include "error_handler.h"

#include <atomic>

int main() {
    Logger::getInstance().setConsoleOutput(false);
    Logger::getInstance().setLogLevel(LogLevel::DEBUG);

    ErrorHandler& handler = ErrorHandler::getInstance();
    std::atomic<int> callback_count{0};
    handler.registerCallback([&](const ErrorInfo&) {
        if (callback_count.fetch_add(1) == 0) {
            // Nested reporting must not reacquire a mutex held by the outer call.
            handler.reportError(ErrorInfo(ErrorCode::ERR_USER_INPUT_INVALID,
                ErrorCategory::USER, "nested report", __FILE__, __LINE__));
        }
    });

    handler.reportError(ErrorInfo(ErrorCode::ERR_NETWORK_TIMEOUT,
        ErrorCategory::NETWORK, "outer report", __FILE__, __LINE__));

    const auto history = handler.getErrorHistory();
    if (callback_count.load() != 2) return 1;
    if (history.size() != 2) return 2;
    if (handler.getErrorCount() != 1) return 3;
    if (handler.hasCriticalError()) return 4;

    handler.setMaxHistorySize(1);
    if (handler.getErrorHistory().size() != 1) return 5;
    return 0;
}
