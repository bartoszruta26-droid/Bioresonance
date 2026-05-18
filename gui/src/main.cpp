/**
 * @file main.cpp
 * @brief Główny punkt wejścia aplikacji GUI
 * 
 * Obsługuje:
 * - Argumenty wiersza poleceń (--verbose, --debug, --log)
 * - Sygnały systemowe (SIGINT, SIGTERM)
 * - Inicjalizację loggera i systemu zdarzeń
 * - Uruchomienie aplikacji GUI
 */

#include "gui_app.h"
#include "logger.h"
#include "error_handler.h"
#include "event_system.h"
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
#endif

static GuiApp* g_app = nullptr;
static std::atomic<bool> g_shutdown_requested{false};

/**
 * @brief Handler sygnałów systemowych - tylko ustawia flagę atomową
 */
void signal_handler(int signum) {
    // Only set atomic flag - avoid non-async-signal-safe functions in handler
    g_shutdown_requested = true;
}

/**
 * @brief Bezpieczne wywołanie cleanup z głównego wątku
 */
void schedule_cleanup(GuiApp* app) {
    if (app) {
        app->stop();
    }
}

/**
 * @brief Inicjalizacja systemów pomocniczych
 */
void initSystems(bool verbose, bool debug, const std::string& log_file) {
    // Konfiguracja loggera
    Logger& logger = Logger::getInstance();
    
    if (debug) {
        logger.setLogLevel(LogLevel::DEBUG);
        logger.setVerbose(true);
    } else if (verbose) {
        logger.setLogLevel(LogLevel::INFO);
        logger.setVerbose(true);
    } else {
        logger.setLogLevel(LogLevel::INFO);
    }
    
    if (!log_file.empty()) {
        logger.setLogFile(log_file);
    }
    
    // Konfiguracja handlera błędów
    ErrorHandler& error_handler = ErrorHandler::getInstance();
    
    // Dodaj callback dla błędów -> publikacja zdarzenia
    error_handler.registerCallback([](const ErrorInfo& error) {
        PUBLISH_EVENT_ASYNC(EventType::ERROR_OCCURRED, "ErrorHandler", error.toString());
    });
    
    // Dodaj handler recovery dla błędów sieciowych
    error_handler.registerRecoveryHandler(ErrorCategory::NETWORK, 
        [](const ErrorInfo& error) -> bool {
            LOG_INFO("Próba recovery po błędzie sieciowym: " + error.message);
            // Tutaj można dodać logikę ponownego łączenia
            return error.retry_count < 3;  // Max 3 próby
        });
    
    // Inicjalizacja systemu zdarzeń
    EventSystem& event_system = EventSystem::getInstance();
    
    // Subskrybuj zdarzenie startu aplikacji
    event_system.subscribe(EventType::APP_START, [](const Event& e) {
        LOG_INFO("Aplikacja uruchomiona: " + e.data);
    });
    
    // Subskrybuj zdarzenia połączenia
    event_system.subscribe(EventType::CONNECT_SUCCESS, [](const Event& e) {
        LOG_INFO("Połączono z urządzeniem: " + e.data);
    });
    
    event_system.subscribe(EventType::CONNECT_FAILED, [](const Event& e) {
        LOG_ERROR("Błąd połączenia: " + e.data);
    });
    
    LOG_DEBUG("Zainicjalizowano systemy pomocnicze");
}

/**
 * @brief Wyświetla informację o użyciu programu
 */
void printUsage(const char* program_name) {
    std::cout << "Użycie: " << program_name << " [opcje]\n"
              << "\nOpcje:\n"
              << "  -i, --ip <adres>       Adres IP urządzenia (domyślnie: 192.168.1.100)\n"
              << "  -p, --port <port>      Port urządzenia (domyślnie: 5001)\n"
              << "  -v, --verbose          Tryb verbose (więcej logów)\n"
              << "  -d, --debug            Tryb debug (szczegółowe logi)\n"
              << "  -l, --log <plik>       Zapisz logi do pliku\n"
              << "  -h, --help             Pokaż tę pomoc\n"
              << "  --version              Pokaż wersję programu\n"
              << "\nPrzykłady:\n"
              << "  " << program_name << " --ip 192.168.1.50 --port 5001\n"
              << "  " << program_name << " --verbose --log bioresonance.log\n"
              << "  " << program_name << " --debug\n";
}

/**
 * @brief Wyświetla informacje o wersji
 */
void printVersion() {
    std::cout << "ResoNet-Nano Biorezonans GUI v1.0.0\n"
              << "Platforma: ";
#ifdef _WIN32
    std::cout << "Windows";
#elif defined(__APPLE__)
    std::cout << "macOS";
#elif defined(__linux__)
    std::cout << "Linux";
#else
    std::cout << "Unknown";
#endif
    std::cout << "\nKompilacja: " << __DATE__ << " " << __TIME__ << "\n";
}

int main(int argc, char* argv[]) {
    // Domyślne wartości
    std::string ip = "192.168.1.100";
    int port = 5001;
    bool verbose = false;
    bool debug = false;
    std::string log_file;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--ip" || arg == "-i") {
            if (i + 1 < argc) {
                ip = argv[++i];
            } else {
                std::cerr << "Błąd: Brak wartości dla opcji --ip\n";
                return 1;
            }
        } else if (arg == "--port" || arg == "-p") {
            if (i + 1 < argc) {
                try {
                    port = std::stoi(argv[++i]);
                    if (port <= 0 || port > 65535) {
                        std::cerr << "Błąd: Nieprawidłowy numer portu\n";
                        return 1;
                    }
                } catch (...) {
                    std::cerr << "Błąd: Nieprawidłowy numer portu\n";
                    return 1;
                }
            } else {
                std::cerr << "Błąd: Brak wartości dla opcji --port\n";
                return 1;
            }
        } else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        } else if (arg == "--debug" || arg == "-d") {
            debug = true;
        } else if (arg == "--log" || arg == "-l") {
            if (i + 1 < argc) {
                log_file = argv[++i];
            } else {
                std::cerr << "Błąd: Brak nazwy pliku dla opcji --log\n";
                return 1;
            }
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--version") {
            printVersion();
            return 0;
        } else {
            std::cerr << "Błąd: Nieznana opcja: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    try {
        // Inicjalizacja systemów
        initSystems(verbose, debug, log_file);
        
        LOG_INFO("=== ResoNet-Nano Biorezonans GUI ===");
        LOG_INFO("Wersja 1.0 - Interfejs graficzny dla systemu biorezonansu");
        LOG_INFO("Parametry: IP=" + ip + ", PORT=" + std::to_string(port));
        if (verbose) LOG_INFO("Tryb: VERBOSE");
        if (debug) LOG_INFO("Tryb: DEBUG");
        if (!log_file.empty()) LOG_INFO("Log file: " + log_file);
        
        // Setup signal handlers
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
#ifdef SIGQUIT
        std::signal(SIGQUIT, signal_handler);
#endif
#ifdef SIGPIPE
        std::signal(SIGPIPE, SIG_IGN);  // Ignore SIGPIPE to prevent crashes
#endif
        
        std::cout << "\n";
        std::cout << "=== ResoNet-Nano Biorezonans GUI ===\n";
        std::cout << "Wersja 1.0 - Interfejs graficzny dla systemu biorezonansu\n";
        std::cout << "\n";
        std::cout << "Domyślne urządzenie: " << ip << ":" << port << "\n";
        std::cout << "\n";
        std::cout << "Sterowanie:\n";
        std::cout << "  - Użyj menu 'Plik' aby połączyć się z urządzeniem\n";
        std::cout << "  - Wybierz końcówkę z listy i skonfiguruj parametry\n";
        std::cout << "  - Przeglądaj bazę częstotliwości w zakładce 'Częstotliwości'\n";
        std::cout << "  - Naciśnij CTRL+C aby zamknąć aplikację\n";
        std::cout << "\n";
        
        // Create and initialize application
        GuiApp app;
        g_app = &app;
        
        // Publikuj zdarzenie startu
        PUBLISH_EVENT(EventType::APP_START, "main", 
                      std::string("IP=") + ip + ":PORT=" + std::to_string(port));
        
        if (!app.init()) {
            LOG_CRITICAL("Błąd inicjalizacji aplikacji!");
            REPORT_ERROR(ErrorCode::ERR_SYSTEM_RESOURCE_UNAVAILABLE, 
                        ErrorCategory::SYSTEM, "Failed to initialize application");
            std::cerr << "Błąd inicjalizacji aplikacji!\n";
            g_app = nullptr;
            return 1;
        }
        
        LOG_INFO("Aplikacja uruchomiona pomyślnie");
        std::cout << "Aplikacja uruchomiona pomyślnie.\n";
        
        // Run the application with signal monitoring
        while (app.isRunning()) {
            app.run();
            
            // Check for shutdown request from signal handler
            if (g_shutdown_requested.load()) {
                LOG_INFO("Otrzymano sygnał zakończenia pracy");
                schedule_cleanup(&app);
                break;
            }
        }
        
        // Publikuj zdarzenie zatrzymania
        PUBLISH_EVENT(EventType::APP_STOP, "main", "Application shutdown");
        
        g_app = nullptr;
        
        // Cleanup
        EventSystem::getInstance().stopProcessingThread();
        
        LOG_INFO("Aplikacja zakończyła działanie");
        std::cout << "Aplikacja zakończyła działanie.\n";
        
        return 0;
        
    } catch (const Exception& e) {
        LOG_CRITICAL(std::string("Wyjątek: ") + e.what());
        std::cerr << "Krytyczny błąd: " << e.what() << "\n";
        g_app = nullptr;
        return 1;
    } catch (const std::exception& e) {
        LOG_CRITICAL(std::string("Wyjątek STL: ") + e.what());
        std::cerr << "Krytyczny błąd: " << e.what() << "\n";
        g_app = nullptr;
        return 1;
    } catch (...) {
        LOG_CRITICAL("Nieznany wyjątek!");
        std::cerr << "Krytyczny błąd: nieznany wyjątek\n";
        g_app = nullptr;
        return 1;
    }
}
