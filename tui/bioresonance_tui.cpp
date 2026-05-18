/**
 * @file bioresonance_tui.cpp
 * @brief Profesjonalny Terminalowy Interfejs Użytkownika dla Systemu Biorezonansu ResoNet-Nano
 * @version 1.0
 * 
 * @description
 * TUI sterujące Arduino Nano z Ethernet HAT poprzez połączenie sieciowe TCP/UDP.
 * Obsługuje liczne końcówki biorezonansowe w trybie pojedynczym i wielokanałowym.
 * 
 * Kompilacja: g++ -std=c++17 -o bioresonance_tui bioresonance_tui.cpp -lncurses
 * Uruchomienie: sudo ./bioresonance_tui <ip_address> [port]
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>

#include <ncurses.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>

// ============================================================================
// KONFIGURACJA SYSTEMU
// ============================================================================

#define DEFAULT_PORT 5001
#define BUFFER_SIZE 1024
#define UI_REFRESH_RATE 100  // ms

// ============================================================================
// STRUKTURY DANYCH
// ============================================================================

/**
 * @brief Typy końcówek biorezonansowych
 */
enum class ProbeType {
    FLAT_COIL,           // Cewka płaska - terapia powierzchniowa
    FERRITE_ROD,         // Cewka ferrytowa - terapia głęboka
    CAPACITIVE_PLATE,    // Płyta kapacytacyjna - aplikacje ogólnoustrojowe
    PEN_APPLICATOR,      // Aplikator punktowy (długopis)
    MAT_APPLICATOR,      // Mata EMF - całe ciało
    LOCAL_PAD,           // Podkładka lokalna
    RING_APPLICATOR,     // Pierścień - kończyny
    COMB_MULTI,          // Kombinacja wielokanałowa
    CUSTOM               // Konfiguracja niestandardowa
};

/**
 * @brief Tryb pracy końcówki
 */
enum class ProbeMode {
    SINGLE,              // Pojedyncza końcówka
    DUAL_INDEPENDENT,    // Dwie niezależne końcówki
    DUAL_SYNC,           // Dwie zsynchronizowane
    MULTI_CHANNEL,       // Wielokanałowa (4+)
    SEQUENTIAL           // Sekwencyjna rotacja
};

/**
 * @brief Typ modulacji
 */
enum class ModulationType {
    NONE = 0,
    AM = 1,              // Modulacja amplitudy
    FM = 2,              // Modulacja częstotliwości
    BURST = 3,           // Impulsowa
    SWEEP = 4            // Przemiatający
};

/**
 * @brief Konfiguracja pojedynczej końcówki
 */
struct ProbeConfig {
    std::string name;
    ProbeType type;
    uint32_t frequency_hz_x100;    // Częstotliwość * 100
    uint32_t duration_sec;         // Czas trwania w sekundach
    uint8_t duty_cycle;            // Wypełnienie PWM 0-100%
    uint16_t intensity_level;      // Intensywność 0-4095
    ModulationType modulation;
    uint8_t modulation_freq_hz;    // Częstotliwość modulacji
    bool enabled;
    int channel_id;
    
    ProbeConfig() : 
        name("Nieaktywna"),
        type(ProbeType::FLAT_COIL),
        frequency_hz_x100(0),
        duration_sec(0),
        duty_cycle(50),
        intensity_level(2048),
        modulation(ModulationType::NONE),
        modulation_freq_hz(1),
        enabled(false),
        channel_id(0) {}
};

/**
 * @brief Pakiet terapeutyczny wysyłany do Arduino
 */
#pragma pack(push, 1)
struct TherapyPacket {
    uint32_t frequency_hz_x100;
    uint32_t duration_sec;
    uint8_t modulation_type;
    uint8_t duty_cycle;
    uint16_t intensity_level;
    uint8_t channel_id;
    uint8_t checksum;
};
#pragma pack(pop)

/**
 * @brief Stan systemu
 */
struct SystemStatus {
    bool connected;
    float temperature;
    bool pwm_running;
    uint32_t current_frequency;
    uint32_t uptime_seconds;
    uint16_t free_memory;
    std::string safety_state;
    std::vector<ProbeConfig> active_probes;
};

// ============================================================================
// KLASY SYSTEMU
// ============================================================================

/**
 * @brief Menadżer końcówek biorezonansowych
 */
class ProbeManager {
private:
    std::map<int, ProbeConfig> probes;
    ProbeMode current_mode;
    int active_channel;
    
public:
    ProbeManager() : current_mode(ProbeMode::SINGLE), active_channel(0) {
        // Inicjalizacja domyślnych konfiguracji
        initializeDefaultProbes();
    }
    
    void initializeDefaultProbes() {
        // Końcówka 1: Cewka płaska
        ProbeConfig p1;
        p1.channel_id = 1;
        p1.name = "Cewka Płaska Główna";
        p1.type = ProbeType::FLAT_COIL;
        p1.frequency_hz_x100 = 72700;  // 727 Hz
        probes[1] = p1;
        
        // Końcówka 2: Cewka ferrytowa
        ProbeConfig p2;
        p2.channel_id = 2;
        p2.name = "Cewka Ferrytowa Punktowa";
        p2.type = ProbeType::FERRITE_ROD;
        p2.frequency_hz_x100 = 10000;  // 10 kHz
        probes[2] = p2;
        
        // Końcówka 3: Płyta kapacytacyjna
        ProbeConfig p3;
        p3.channel_id = 3;
        p3.name = "Płyta Kapacytacyjna";
        p3.type = ProbeType::CAPACITIVE_PLATE;
        p3.frequency_hz_x100 = 5000;   // 5 kHz
        probes[3] = p3;
        
        // Końcówka 4: Aplikator punktowy
        ProbeConfig p4;
        p4.channel_id = 4;
        p4.name = "Aplikator Punktowy";
        p4.type = ProbeType::PEN_APPLICATOR;
        p4.frequency_hz_x100 = 25000;  // 25 kHz
        probes[4] = p4;
        
        // Końcówka 5: Mata EMF
        ProbeConfig p5;
        p5.channel_id = 5;
        p5.name = "Mata EMF Całe Ciało";
        p5.type = ProbeType::MAT_APPLICATOR;
        p5.frequency_hz_x100 = 7830;   // 78.3 Hz (Schumann)
        probes[5] = p5;
        
        // Końcówka 6: Podkładka lokalna
        ProbeConfig p6;
        p6.channel_id = 6;
        p6.name = "Podkładka Lokalna";
        p6.type = ProbeType::LOCAL_PAD;
        p6.frequency_hz_x100 = 100000; // 1 kHz
        probes[6] = p6;
        
        // Końcówka 7: Pierścień
        ProbeConfig p7;
        p7.channel_id = 7;
        p7.name = "Pierścień Kończyna";
        p7.type = ProbeType::RING_APPLICATOR;
        p7.frequency_hz_x100 = 50000;  // 500 Hz
        probes[7] = p7;
        
        // Końcówka 8: Konfiguracja niestandardowa
        ProbeConfig p8;
        p8.channel_id = 8;
        p8.name = "Konfiguracja Niestandardowa";
        p8.type = ProbeType::CUSTOM;
        p8.frequency_hz_x100 = 1000;   // 10 Hz
        probes[8] = p8;
    }
    
    ProbeConfig* getProbe(int channel_id) {
        auto it = probes.find(channel_id);
        return (it != probes.end()) ? &it->second : nullptr;
    }
    
    std::vector<ProbeConfig*> getAllProbes() {
        std::vector<ProbeConfig*> result;
        for (auto& pair : probes) {
            if (pair.second.enabled) {
                result.push_back(&pair.second);
            }
        }
        return result;
    }
    
    void setProbeMode(ProbeMode mode) {
        current_mode = mode;
    }
    
    ProbeMode getProbeMode() const {
        return current_mode;
    }
    
    void enableProbe(int channel_id, bool enable) {
        ProbeConfig* probe = getProbe(channel_id);
        if (probe) {
            probe->enabled = enable;
        }
    }
    
    bool isProbeEnabled(int channel_id) {
        ProbeConfig* probe = getProbe(channel_id);
        return probe && probe->enabled;
    }
    
    std::string getProbeTypeName(ProbeType type) {
        switch (type) {
            case ProbeType::FLAT_COIL: return "Cewka Płaska";
            case ProbeType::FERRITE_ROD: return "Cewka Ferrytowa";
            case ProbeType::CAPACITIVE_PLATE: return "Płyta Kapacytacyjna";
            case ProbeType::PEN_APPLICATOR: return "Aplikator Punktowy";
            case ProbeType::MAT_APPLICATOR: return "Mata EMF";
            case ProbeType::LOCAL_PAD: return "Podkładka Lokalna";
            case ProbeType::RING_APPLICATOR: return "Pierścień";
            case ProbeType::COMB_MULTI: return "Kombinacja";
            case ProbeType::CUSTOM: return "Niestandardowa";
            default: return "Nieznana";
        }
    }
    
    std::string getModeName(ProbeMode mode) {
        switch (mode) {
            case ProbeMode::SINGLE: return "Pojedyncza";
            case ProbeMode::DUAL_INDEPENDENT: return "Dual Niezależny";
            case ProbeMode::DUAL_SYNC: return "Dual Sync";
            case ProbeMode::MULTI_CHANNEL: return "Wielokanałowa";
            case ProbeMode::SEQUENTIAL: return "Sekwencyjna";
            default: return "Nieznany";
        }
    }
};

/**
 * @brief Klient sieciowy komunikujący się z Arduino
 */
class NetworkClient {
private:
    int socket_fd;
    struct sockaddr_in server_addr;
    std::string ip_address;
    int port;
    std::atomic<bool> connected;
    std::mutex tx_mutex;
    
public:
    NetworkClient() : socket_fd(-1), connected(false) {}
    
    ~NetworkClient() {
        disconnect();
    }
    
    bool connect(const std::string& ip, int port_num) {
        ip_address = ip;
        port = port_num;
        
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            return false;
        }
        
        // Non-blocking connect
        int flags = fcntl(socket_fd, F_GETFL, 0);
        fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
        
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
            close(socket_fd);
            socket_fd = -1;
            return false;
        }
        
        int result = ::connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (result < 0 && errno != EINPROGRESS) {
            close(socket_fd);
            socket_fd = -1;
            return false;
        }
        
        // Wait for connection
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(socket_fd, &fds);
        
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        result = select(socket_fd + 1, NULL, &fds, NULL, &tv);
        if (result <= 0) {
            close(socket_fd);
            socket_fd = -1;
            return false;
        }
        
        // Check for connection errors using SO_ERROR
        int so_error = 0;
        socklen_t len = sizeof(so_error);
        if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0 || so_error != 0) {
            close(socket_fd);
            socket_fd = -1;
            return false;
        }
        
        // Set blocking mode
        fcntl(socket_fd, F_SETFL, flags);
        
        connected = true;
        return true;
    }
    
    void disconnect() {
        if (socket_fd >= 0) {
            close(socket_fd);
            socket_fd = -1;
        }
        connected = false;
    }
    
    bool isConnected() const {
        return connected && socket_fd >= 0;
    }
    
    bool sendTherapyPacket(const TherapyPacket& packet) {
        if (!isConnected()) return false;
        
        std::lock_guard<std::mutex> lock(tx_mutex);
        
        ssize_t sent = send(socket_fd, &packet, sizeof(packet), 0);
        return (sent == sizeof(packet));
    }
    
    bool sendCommand(const std::string& cmd) {
        if (!isConnected()) return false;
        
        std::lock_guard<std::mutex> lock(tx_mutex);
        ssize_t sent = send(socket_fd, cmd.c_str(), cmd.length(), 0);
        return (sent > 0);
    }
    
    std::string receiveData(int timeout_ms = 100) {
        if (!isConnected()) return "";
        
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(socket_fd, &fds);
        
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        
        int result = select(socket_fd + 1, &fds, NULL, NULL, &tv);
        if (result <= 0) return "";
        
        char buffer[BUFFER_SIZE];
        ssize_t received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (received > 0) {
            buffer[received] = '\0';
            return std::string(buffer, received);
        }
        
        return "";
    }
};

/**
 * @brief Główna klasa aplikacji TUI
 */
class BioresonanceTUI {
private:
    WINDOW* main_win;
    WINDOW* header_win;
    WINDOW* menu_win;
    WINDOW* probes_win;
    WINDOW* status_win;
    WINDOW* log_win;
    
    NetworkClient network;
    ProbeManager probe_manager;
    
    SystemStatus status;
    std::vector<std::string> log_messages;
    int selected_probe;
    int current_menu;
    bool running;
    
    std::string device_ip;
    int device_port;
    
public:
    BioresonanceTUI(const std::string& ip, int port) 
        : selected_probe(1), current_menu(0), running(true), 
          device_ip(ip), device_port(port) {
        
        status.connected = false;
        status.temperature = 0.0f;
        status.pwm_running = false;
        status.current_frequency = 0;
        status.uptime_seconds = 0;
        status.free_memory = 0;
        status.safety_state = "UNKNOWN";
        
        // Initialize ncurses
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        
        // Colors
        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Status OK
            init_pair(2, COLOR_RED, COLOR_BLACK);     // Error
            init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Warning
            init_pair(4, COLOR_CYAN, COLOR_BLACK);    // Info
            init_pair(5, COLOR_WHITE, COLOR_BLUE);    // Selected
            init_pair(6, COLOR_MAGENTA, COLOR_BLACK); // Header
        }
        
        createWindows();
        
        // Try to connect
        connectToDevice();
    }
    
    ~BioresonanceTUI() {
        network.disconnect();
        delwin(main_win);
        delwin(header_win);
        delwin(menu_win);
        delwin(probes_win);
        delwin(status_win);
        delwin(log_win);
        endwin();
    }
    
    void createWindows() {
        int height, width;
        getmaxyx(stdscr, height, width);
        
        // Main window
        main_win = newwin(height, width, 0, 0);
        box(main_win, 0, 0);
        
        // Header
        header_win = newwin(3, width - 2, 1, 1);
        
        // Menu
        menu_win = newwin(10, width / 4, 5, 1);
        box(menu_win, 0, 0);
        
        // Probes
        probes_win = newwin(height - 18, width * 3 / 4 - 2, 5, width / 4 + 1);
        box(probes_win, 0, 0);
        
        // Status
        status_win = newwin(4, width - 2, height - 12, 1);
        box(status_win, 0, 0);
        
        // Log
        log_win = newwin(6, width - 2, height - 7, 1);
        box(log_win, 0, 0);
    }
    
    void connectToDevice() {
        addLog("Łączenie z urządzeniem...");
        if (network.connect(device_ip, device_port)) {
            status.connected = true;
            addLog("Połączono z " + device_ip + ":" + std::to_string(device_port));
            requestStatus();
        } else {
            status.connected = false;
            addLog("Błąd połączenia!");
        }
    }
    
    void addLog(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%H:%M:%S");
        log_messages.insert(log_messages.begin(), "[" + ss.str() + "] " + message);
        if (log_messages.size() > 50) {
            log_messages.pop_back();
        }
    }
    
    void requestStatus() {
        network.sendCommand("s\n");
    }
    
    void parseStatus(const std::string& data) {
        // Parse status response from Arduino
        // Format: === System Status ===\nUptime: 125s\n...
        std::istringstream iss(data);
        std::string line;
        
        while (std::getline(iss, line)) {
            if (line.find("Uptime:") != std::string::npos) {
                unsigned long uptime_tmp;
                if (sscanf(line.c_str(), "Uptime: %lu", &uptime_tmp) == 1) {
                    status.uptime_seconds = static_cast<uint32_t>(uptime_tmp);
                }
            } else if (line.find("Free Memory:") != std::string::npos) {
                sscanf(line.c_str(), "Free Memory: %hu bytes", &status.free_memory);
            } else if (line.find("Temperature:") != std::string::npos) {
                sscanf(line.c_str(), "Temperature: %f C", &status.temperature);
            } else if (line.find("PWM Running:") != std::string::npos) {
                status.pwm_running = (line.find("YES") != std::string::npos);
            } else if (line.find("Frequency:") != std::string::npos) {
                sscanf(line.c_str(), "Frequency: %lu Hz", &status.current_frequency);
            } else if (line.find("Network:") != std::string::npos) {
                status.connected = (line.find("CONNECTED") != std::string::npos);
            } else if (line.find("Safety State:") != std::string::npos) {
                char safety_buf[64];
                if (sscanf(line.c_str(), "Safety State: %63s", safety_buf) == 1) {
                    status.safety_state = safety_buf;
                }
            }
        }
    }
    
    void drawHeader() {
        werase(header_win);
        
        wattron(header_win, COLOR_PAIR(6) | A_BOLD);
        mvwprintw(header_win, 0, 1, " RESONET-NANO TUI v1.0 ");
        wattroff(header_win, COLOR_PAIR(6) | A_BOLD);
        
        mvwprintw(header_win, 1, 1, "Device: %s:%d", device_ip.c_str(), device_port);
        
        if (status.connected) {
            wattron(header_win, COLOR_PAIR(1));
            mvwprintw(header_win, 1, 30, "[POŁĄCZONO]");
            wattroff(header_win, COLOR_PAIR(1));
        } else {
            wattron(header_win, COLOR_PAIR(2));
            mvwprintw(header_win, 1, 30, "[ROZŁĄCZONO]");
            wattroff(header_win, COLOR_PAIR(2));
        }
        
        wrefresh(header_win);
    }
    
    void drawMenu() {
        werase(menu_win);
        mvwprintw(menu_win, 0, 2, " MENU GŁÓWNE ");
        
        const char* menu_items[] = {
            "1. Konfiguracja Końcówek",
            "2. Tryby Pracy",
            "3. Start Terapii",
            "4. Stop Terapii",
            "5. Status Systemu",
            "6. Kalibracja",
            "7. Presety",
            "8. Wyjście"
        };
        
        for (int i = 0; i < 8; i++) {
            if (current_menu == i) {
                wattron(menu_win, COLOR_PAIR(5));
                mvwprintw(menu_win, i + 2, 2, ">%s<", menu_items[i]);
                wattroff(menu_win, COLOR_PAIR(5));
            } else {
                mvwprintw(menu_win, i + 2, 3, "%s", menu_items[i]);
            }
        }
        
        wrefresh(menu_win);
    }
    
    void drawProbes() {
        werase(probes_win);
        mvwprintw(probes_win, 0, 2, " KONFIGURACJA KOŃCÓWEK ");
        
        int y = 2;
        for (int i = 1; i <= 8; i++) {
            ProbeConfig* probe = probe_manager.getProbe(i);
            if (probe) {
                std::string status_str = probe->enabled ? "[AKTYWNA]" : "[NIEAKTYWNA]";
                int color = probe->enabled ? 1 : 3;
                
                if (selected_probe == i) {
                    wattron(probes_win, COLOR_PAIR(5));
                } else {
                    wattron(probes_win, COLOR_PAIR(color));
                }
                
                mvwprintw(probes_win, y++, 2, "Kanał %d: %s %s", 
                         i, probe->name.c_str(), status_str.c_str());
                
                if (probe->enabled) {
                    mvwprintw(probes_win, y++, 4, "Freq: %.2f Hz | Duty: %d%% | Int: %d",
                             probe->frequency_hz_x100 / 100.0,
                             probe->duty_cycle,
                             probe->intensity_level);
                    mvwprintw(probes_win, y++, 4, "Typ: %s | Mod: %s",
                             probe_manager.getProbeTypeName(probe->type).c_str(),
                             getModulationName(probe->modulation).c_str());
                }
                
                wattroff(probes_win, COLOR_PAIR(5));
                wattroff(probes_win, COLOR_PAIR(color));
                y++;
            }
        }
        
        wrefresh(probes_win);
    }
    
    void drawStatus() {
        werase(status_win);
        mvwprintw(status_win, 0, 2, " STATUS SYSTEMU ");
        
        mvwprintw(status_win, 1, 2, "Temp: %.1f°C | Mem: %d B | Uptime: %lu s",
                 status.temperature, status.free_memory, status.uptime_seconds);
        
        if (status.pwm_running) {
            wattron(status_win, COLOR_PAIR(1));
            mvwprintw(status_win, 2, 2, "PWM: ACTIVE | Freq: %lu Hz", status.current_frequency);
            wattroff(status_win, COLOR_PAIR(1));
        } else {
            mvwprintw(status_win, 2, 2, "PWM: STOPPED");
        }
        
        wrefresh(status_win);
    }
    
    void drawLog() {
        werase(log_win);
        mvwprintw(log_win, 0, 2, " DZIENNIK ZDARZEŃ ");
        
        for (size_t i = 0; i < std::min(log_messages.size(), (size_t)4); i++) {
            mvwprintw(log_win, i + 1, 2, "%s", log_messages[i].c_str());
        }
        
        wrefresh(log_win);
    }
    
    std::string getModulationName(ModulationType mod) {
        switch (mod) {
            case ModulationType::NONE: return "Brak";
            case ModulationType::AM: return "AM";
            case ModulationType::FM: return "FM";
            case ModulationType::BURST: return "Burst";
            case ModulationType::SWEEP: return "Sweep";
            default: return "Nieznana";
        }
    }
    
    void refreshDisplay() {
        drawHeader();
        drawMenu();
        drawProbes();
        drawStatus();
        drawLog();
        refresh();
    }
    
    void handleInput() {
        int ch = getch();
        
        switch (ch) {
            case KEY_UP:
                if (current_menu > 0) current_menu--;
                break;
            case KEY_DOWN:
                if (current_menu < 7) current_menu++;
                break;
            case KEY_LEFT:
                if (selected_probe > 1) selected_probe--;
                break;
            case KEY_RIGHT:
                if (selected_probe < 8) selected_probe++;
                break;
            case 'q':
            case 'Q':
                running = false;
                break;
            case '1':
                configureProbe();
                break;
            case '2':
                selectProbeMode();
                break;
            case '3':
                startTherapy();
                break;
            case '4':
                stopTherapy();
                break;
            case '5':
                requestStatus();
                addLog("Żądanie statusu...");
                break;
            case 'e':
                toggleProbe(selected_probe);
                break;
            case 'f':
                editFrequency();
                break;
            case 'i':
                editIntensity();
                break;
            case 'm':
                editModulation();
                break;
        }
    }
    
    void configureProbe() {
        ProbeConfig* probe = probe_manager.getProbe(selected_probe);
        if (!probe) return;
        
        addLog("Konfiguracja: " + probe->name);
        
        // Show configuration submenu
        echo();
        mvprintw(20, 2, "Nowa częstotliwość (Hz): ");
        refresh();
        
        char input[32];
        getnstr(input, sizeof(input));
        
        float freq = atof(input);
        if (freq > 0) {
            probe->frequency_hz_x100 = (uint32_t)(freq * 100);
            addLog("Ustawiono freq: " + std::to_string(freq) + " Hz");
        }
        
        noecho();
    }
    
    void toggleProbe(int channel) {
        ProbeConfig* probe = probe_manager.getProbe(channel);
        if (!probe) return;
        
        probe->enabled = !probe->enabled;
        std::string status_str = probe->enabled ? "AKTYWNA" : "NIEAKTYWNA";
        addLog("Kanał " + std::to_string(channel) + ": " + status_str);
        
        // Send update to Arduino if enabled
        if (probe->enabled && network.isConnected()) {
            sendProbeConfig(probe);
        }
    }
    
    void sendProbeConfig(ProbeConfig* probe) {
        if (!network.isConnected()) return;
        
        TherapyPacket packet;
        packet.frequency_hz_x100 = probe->frequency_hz_x100;
        packet.duration_sec = probe->duration_sec;
        packet.modulation_type = (uint8_t)probe->modulation;
        packet.duty_cycle = probe->duty_cycle;
        packet.intensity_level = probe->intensity_level;
        packet.channel_id = probe->channel_id;
        packet.checksum = calculateChecksum(packet);
        
        if (network.sendTherapyPacket(packet)) {
            addLog("Wysłano konfigurację kanału " + std::to_string(probe->channel_id));
        } else {
            addLog("Błąd wysyłania konfiguracji!");
        }
    }
    
    uint8_t calculateChecksum(const TherapyPacket& packet) {
        uint8_t* data = (uint8_t*)&packet;
        uint8_t sum = 0;
        for (size_t i = 0; i < sizeof(packet) - 1; i++) {
            sum += data[i];
        }
        return sum;
    }
    
    void selectProbeMode() {
        addLog("Wybór trybu pracy końcówek");
        // Implementation for probe mode selection
    }
    
    void startTherapy() {
        addLog("Rozpoczynanie terapii...");
        auto active_probes = probe_manager.getAllProbes();
        
        if (active_probes.empty()) {
            addLog("BRAK AKTYWNYCH KOŃCÓWEK!");
            return;
        }
        
        for (auto* probe : active_probes) {
            sendProbeConfig(probe);
        }
        
        network.sendCommand("t\n");  // Test command
        addLog("Terapia rozpoczęta!");
    }
    
    void stopTherapy() {
        addLog("Zatrzymywanie terapii...");
        network.sendCommand("x\n");
        addLog("Terapia zatrzymana");
    }
    
    void editFrequency() {
        ProbeConfig* probe = probe_manager.getProbe(selected_probe);
        if (!probe || !probe->enabled) {
            addLog("Wybierz aktywną końcówkę!");
            return;
        }
        
        echo();
        mvprintw(20, 2, "Częstotliwość (Hz): ");
        refresh();
        
        char input[32];
        getnstr(input, sizeof(input));
        
        float freq = atof(input);
        if (freq > 0) {
            probe->frequency_hz_x100 = (uint32_t)(freq * 100);
            addLog("Freq: " + std::to_string(freq) + " Hz");
            sendProbeConfig(probe);
        }
        
        noecho();
    }
    
    void editIntensity() {
        ProbeConfig* probe = probe_manager.getProbe(selected_probe);
        if (!probe || !probe->enabled) return;
        
        echo();
        mvprintw(20, 2, "Intensywność (0-4095): ");
        refresh();
        
        char input[32];
        getnstr(input, sizeof(input));
        
        int intensity = atoi(input);
        if (intensity >= 0 && intensity <= 4095) {
            probe->intensity_level = (uint16_t)intensity;
            addLog("Intensity: " + std::to_string(intensity));
            sendProbeConfig(probe);
        }
        
        noecho();
    }
    
    void editModulation() {
        ProbeConfig* probe = probe_manager.getProbe(selected_probe);
        if (!probe || !probe->enabled) return;
        
        int mod = (int)probe->modulation + 1;
        if (mod > 4) mod = 0;
        probe->modulation = (ModulationType)mod;
        
        addLog("Modulacja: " + getModulationName(probe->modulation));
        sendProbeConfig(probe);
    }
    
    void run() {
        while (running) {
            refreshDisplay();
            handleInput();
            
            // Receive and parse status
            std::string data = network.receiveData(50);
            if (!data.empty()) {
                parseStatus(data);
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(UI_REFRESH_RATE));
        }
    }
};

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char* argv[]) {
    std::string ip = "192.168.1.100";
    int port = DEFAULT_PORT;
    
    if (argc > 1) {
        ip = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }
    
    std::cout << "=== ResoNet-Nano Bioresonance TUI ===" << std::endl;
    std::cout << "Connecting to: " << ip << ":" << port << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  UP/DOWN - Menu navigation" << std::endl;
    std::cout << "  LEFT/RIGHT - Probe selection" << std::endl;
    std::cout << "  1-8 - Quick actions" << std::endl;
    std::cout << "  E - Toggle probe" << std::endl;
    std::cout << "  F - Edit frequency" << std::endl;
    std::cout << "  I - Edit intensity" << std::endl;
    std::cout << "  M - Edit modulation" << std::endl;
    std::cout << "  Q - Quit" << std::endl;
    std::cout << std::endl;
    
    try {
        BioresonanceTUI tui(ip, port);
        tui.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
