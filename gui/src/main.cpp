/**
 * @file main.cpp
 * @brief Główny punkt wejścia aplikacji GUI
 */

#include "gui_app.h"
#include <iostream>
#include <csignal>

#ifdef _WIN32
    #include <windows.h>
#endif

static GuiApp* g_app = nullptr;

void signal_handler(int signum) {
    std::cout << "\nOtrzymano sygnał " << signum << ", zamykanie aplikacji..." << std::endl;
    if (g_app) {
        g_app->stop();
    }
}

int main(int argc, char* argv[]) {
    // Setup signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    std::cout << "=== ResoNet-Nano Biorezonans GUI ===" << std::endl;
    std::cout << "Wersja 1.0 - Interfejs graficzny dla systemu biorezonansu" << std::endl;
    std::cout << std::endl;
    
    // Parse command line arguments
    std::string ip = "192.168.1.100";
    int port = 5001;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--ip" || arg == "-i") {
            if (i + 1 < argc) {
                ip = argv[++i];
            }
        } else if (arg == "--port" || arg == "-p") {
            if (i + 1 < argc) {
                port = std::atoi(argv[++i]);
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Użycie: " << argv[0] << " [opcje]" << std::endl;
            std::cout << "Opcje:" << std::endl;
            std::cout << "  -i, --ip <adres>     Adres IP urządzenia (domyślnie: 192.168.1.100)" << std::endl;
            std::cout << "  -p, --port <port>    Port urządzenia (domyślnie: 5001)" << std::endl;
            std::cout << "  -h, --help           Pokaż tę pomoc" << std::endl;
            return 0;
        }
    }
    
    // Create and initialize application
    GuiApp app;
    g_app = &app;
    
    if (!app.init()) {
        std::cerr << "Błąd inicjalizacji aplikacji!" << std::endl;
        return 1;
    }
    
    std::cout << "Aplikacja uruchomiona pomyślnie." << std::endl;
    std::cout << "Domyślne urządzenie: " << ip << ":" << port << std::endl;
    std::cout << std::endl;
    std::cout << "Sterowanie:" << std::endl;
    std::cout << "  - Użyj menu 'Plik' aby połączyć się z urządzeniem" << std::endl;
    std::cout << "  - Wybierz końcówkę z listy i skonfiguruj parametry" << std::endl;
    std::cout << "  - Przeglądaj bazę częstotliwości w zakładce 'Częstotliwości'" << std::endl;
    std::cout << "  - Naciśnij CTRL+C aby zamknąć aplikację" << std::endl;
    std::cout << std::endl;
    
    // Run the application
    app.run();
    
    g_app = nullptr;
    std::cout << "Aplikacja zakończyła działanie." << std::endl;
    
    return 0;
}
