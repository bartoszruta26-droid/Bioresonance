/**
 * @file gui_app.cpp
 * @brief Implementacja głównej aplikacji GUI
 */

#include "gui_app.h"
#include <iostream>
#include <chrono>
#include <thread>

// SDL2 and ImGui includes
#include "SDL.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_stdlib.h"

#ifdef _WIN32
    #include <winsock2.h>
#endif

GuiApp::GuiApp() 
    : window(nullptr), renderer(nullptr), selected_probe(1), running(false),
      device_ip("192.168.1.100"), device_port(5001),
      show_frequency_browser(false), show_probe_settings(true), 
      show_system_status(true), current_tab(0) {
}

GuiApp::~GuiApp() {
    cleanupSDL();
}

bool GuiApp::initSDL() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create window
    window = SDL_CreateWindow("ResoNet-Nano Biorezonans GUI",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              1280, 720, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    
    // Setup style
    ImGui::StyleColorsDark();
    
    addLog("Zainicjalizowano SDL2 i Dear ImGui");
    return true;
}

void GuiApp::cleanupSDL() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    SDL_Quit();
}

void GuiApp::addLog(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    char time_buf[64];
    struct tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time);
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &tm_buf);
#else
    localtime_r(&time, &tm_buf);
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &tm_buf);
#endif
    
    log_messages.push_back(std::string("[") + time_buf + "] " + message);
    
    // Keep only last 100 messages
    if (log_messages.size() > 100) {
        log_messages.erase(log_messages.begin());
    }
}

bool GuiApp::init() {
    if (!initSDL()) {
        return false;
    }
    
    // Load frequency database
    if (frequency_loader.loadFromFile("frequencies.md")) {
        addLog("Załadowano bazę częstotliwości: " + std::to_string(frequency_loader.getDatabaseSize()) + " wpisów");
    } else if (frequency_loader.loadFromFile("../frequencies.md")) {
        addLog("Załadowano bazę częstotliwości: " + std::to_string(frequency_loader.getDatabaseSize()) + " wpisów");
    } else {
        addLog("Nie udało się załadować frequencies.md");
    }
    
    addLog("Aplikacja gotowa. Podłącz urządzenie aby rozpocząć.");
    return true;
}

bool GuiApp::connectToDevice(const std::string& ip, int port) {
    addLog("Łączenie z " + ip + ":" + std::to_string(port) + "...");
    
    if (network.connect(ip, port)) {
        status.connected = true;
        device_ip = ip;
        device_port = port;
        addLog("Połączono pomyślnie!");
        return true;
    } else {
        status.connected = false;
        addLog("Błąd połączenia!");
        return false;
    }
}

void GuiApp::disconnectFromDevice() {
    network.disconnect();
    status.connected = false;
    addLog("Rozłączono z urządzeniem");
}

void GuiApp::onProbeSelected(int probe_id) {
    selected_probe = probe_id;
    addLog("Wybrano końcówkę: " + std::to_string(probe_id));
    
    // Update status to reflect the selected probe
    ProbeConfig* probe = probe_manager.getProbe(probe_id);
    if (probe) {
        status.current_frequency = probe->frequency_hz_x100 / 100;
    }
}

void GuiApp::onProbeEnabledChanged(int probe_id, bool enabled) {
    probe_manager.enableProbe(probe_id, enabled);
    addLog("Końcówka " + std::to_string(probe_id) + (enabled ? " włączona" : " wyłączona"));
}

void GuiApp::onFrequencySelected(uint32_t frequency_hz) {
    ProbeConfig* probe = probe_manager.getProbe(selected_probe);
    if (probe) {
        probe->frequency_hz_x100 = frequency_hz * 100;
        status.current_frequency = frequency_hz;  // Update status
        addLog("Ustawiono częstotliwość: " + std::to_string(frequency_hz) + " Hz");
    }
}

void GuiApp::onStartTherapy() {
    if (!status.connected) {
        addLog("Błąd: Nie podłączono do urządzenia!");
        return;
    }
    
    ProbeConfig* probe = probe_manager.getProbe(selected_probe);
    if (probe && probe->enabled) {
        TherapyPacket packet;
        packet.frequency_hz_x100 = probe->frequency_hz_x100;
        packet.duration_sec = probe->duration_sec;
        packet.modulation_type = (uint8_t)probe->modulation;
        packet.duty_cycle = probe->duty_cycle;
        packet.intensity_level = probe->intensity_level;
        packet.channel_id = probe->channel_id;
        packet.checksum = calculateChecksum(packet);  // Calculate proper checksum
        
        if (network.sendTherapyPacket(packet)) {
            status.pwm_running = true;
            addLog("Rozpoczęto terapię na kanale " + std::to_string(probe->channel_id));
        } else {
            addLog("Błąd wysyłania pakietu terapeutycznego");
        }
    }
}

uint8_t GuiApp::calculateChecksum(const TherapyPacket& packet) {
    // Safe checksum calculation using explicit field access instead of raw memory
    uint8_t checksum = 0;
    checksum += static_cast<uint8_t>(packet.frequency_hz_x100 & 0xFF);
    checksum += static_cast<uint8_t>((packet.frequency_hz_x100 >> 8) & 0xFF);
    checksum += static_cast<uint8_t>((packet.frequency_hz_x100 >> 16) & 0xFF);
    checksum += static_cast<uint8_t>((packet.frequency_hz_x100 >> 24) & 0xFF);
    
    checksum += static_cast<uint8_t>(packet.duration_sec & 0xFF);
    checksum += static_cast<uint8_t>((packet.duration_sec >> 8) & 0xFF);
    checksum += static_cast<uint8_t>((packet.duration_sec >> 16) & 0xFF);
    checksum += static_cast<uint8_t>((packet.duration_sec >> 24) & 0xFF);
    
    checksum += packet.modulation_type;
    checksum += packet.duty_cycle;
    
    checksum += static_cast<uint8_t>(packet.intensity_level & 0xFF);
    checksum += static_cast<uint8_t>((packet.intensity_level >> 8) & 0xFF);
    
    checksum += packet.channel_id;
    
    return checksum;
}

void GuiApp::onStopTherapy() {
    status.pwm_running = false;
    network.sendCommand("STOP");
    addLog("Zatrzymano terapię");
}

void GuiApp::renderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Plik")) {
            if (ImGui::MenuItem("Połącz", "Ctrl+O")) {
                // Connection dialog handled in status panel
            }
            if (ImGui::MenuItem("Rozłącz", "Ctrl+D")) {
                disconnectFromDevice();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Wyjście", "Alt+F4")) {
                running = false;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Widok")) {
            ImGui::MenuItem("Przeglądarka częstotliwości", nullptr, &show_frequency_browser);
            ImGui::MenuItem("Ustawienia końcówek", nullptr, &show_probe_settings);
            ImGui::MenuItem("Status systemu", nullptr, &show_system_status);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Pomoc")) {
            if (ImGui::MenuItem("O programie")) {
                ImGui::OpenPopup("About");
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
    
    // About popup
    if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("ResoNet-Nano Biorezonans GUI");
        ImGui::Separator();
        ImGui::Text("Wersja 1.0");
        ImGui::Text("Interfejs GUI dla systemu biorezonansu");
        ImGui::Spacing();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void GuiApp::renderMainTabs() {
    ImGui::BeginTabBar("MainTabs");
    
    if (ImGui::BeginTabItem("Panel główny")) {
        renderProbePanel();
        ImGui::EndTabItem();
    }
    
    if (ImGui::BeginTabItem("Częstotliwości")) {
        renderFrequencyBrowser();
        ImGui::EndTabItem();
    }
    
    if (ImGui::BeginTabItem("Status")) {
        renderStatusPanel();
        ImGui::EndTabItem();
    }
    
    ImGui::EndTabBar();
}

void GuiApp::renderProbePanel() {
    ImGui::BeginChild("ProbeList", ImVec2(250, 0), true);
    ImGui::Text("Końcówki");
    ImGui::Separator();
    
    // Use const reference to avoid copying the entire map
    const auto& probes_map = probe_manager.getProbesMap();
    for (const auto& [id, config] : probes_map) {
        bool is_selected = (selected_probe == id);
        
        if (ImGui::Selectable(config.name.c_str(), is_selected)) {
            onProbeSelected(id);
        }
        
        // Show enabled checkbox - use fixed-size buffer with bounds checking
        char checkbox_label[32];
        snprintf(checkbox_label, sizeof(checkbox_label), "##enable_%d", id);
        bool enabled = config.enabled;
        if (ImGui::Checkbox(checkbox_label, &enabled)) {
            onProbeEnabledChanged(id, enabled);
        }
        ImGui::SameLine();
        
        // Show type icon
        ImGui::Text("%s", probe_manager.getProbeTypeName(config.type).c_str());
    }
    
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    ImGui::BeginChild("ProbeSettings", ImVec2(0, 0), true);
    ImGui::Text("Ustawienia wybranej końcówki");
    ImGui::Separator();
    
    ProbeConfig* probe = probe_manager.getProbe(selected_probe);
    if (probe) {
        ImGui::InputText("Nazwa", &probe->name);
        
        float freq_hz = static_cast<float>(probe->frequency_hz_x100) / 100.0f;
        if (ImGui::SliderFloat("Częstotliwość (Hz)", &freq_hz, 100.0f, 1000000.0f)) {
            probe->frequency_hz_x100 = static_cast<uint32_t>(freq_hz * 100.0f + 0.5f);
        }
        
        int duration_sec = static_cast<int>(probe->duration_sec);
        if (ImGui::SliderInt("Czas trwania (s)", &duration_sec, 1, 3600)) {
            probe->duration_sec = static_cast<uint32_t>(duration_sec);
        }
        
        int duty_cycle = static_cast<int>(probe->duty_cycle);
        if (ImGui::SliderInt("Wypełnienie PWM (%)", &duty_cycle, 0, 100)) {
            probe->duty_cycle = static_cast<uint8_t>(duty_cycle);
        }
        
        int intensity = static_cast<int>(probe->intensity_level);
        if (ImGui::SliderInt("Intensywność", &intensity, 0, 4095)) {
            probe->intensity_level = static_cast<uint16_t>(intensity);
        }
        
        int mod_freq = static_cast<int>(probe->modulation_freq_hz);
        if (probe->modulation != ModulationType::NONE) {
            if (ImGui::SliderInt("Częst. modulacji (Hz)", &mod_freq, 1, 100)) {
                probe->modulation_freq_hz = static_cast<uint8_t>(mod_freq);
            }
        }
        
        // Modulation type combo
        const char* modulation_types[] = {"Brak", "AM", "FM", "BURST", "SWEEP"};
        int current_mod = (int)probe->modulation;
        if (ImGui::Combo("Modulacja", &current_mod, modulation_types, 5)) {
            probe->modulation = (ModulationType)current_mod;
        }
        
        ImGui::Separator();
        
        // Therapy control buttons
        if (!status.pwm_running) {
            if (ImGui::Button("Rozpocznij terapię", ImVec2(200, 40))) {
                onStartTherapy();
            }
        } else {
            if (ImGui::Button("Zatrzymaj terapię", ImVec2(200, 40))) {
                onStopTherapy();
            }
        }
    }
    
    ImGui::EndChild();
}

void GuiApp::renderFrequencyBrowser() {
    static char search_buffer[256] = "";
    std::vector<FrequencyEntry> filtered_results;  // Non-static to avoid thread safety issues
    bool search_triggered = false;
    
    ImGui::Text("Przeglądarka częstotliwości");
    ImGui::Separator();
    
    // Search bar
    if (ImGui::InputText("##Search", search_buffer, sizeof(search_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        filtered_results = frequency_loader.searchByDisease(search_buffer);
        search_triggered = true;
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Szukaj choroby")) {
        filtered_results = frequency_loader.searchByDisease(search_buffer);
        search_triggered = true;
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Pokaż wszystkie")) {
        filtered_results.clear();
        search_triggered = false;
    }
    
    ImGui::Separator();
    
    // Results table
    if (ImGui::BeginTable("FreqTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Częstotliwość (Hz)");
        ImGui::TableSetupColumn("Kategoria");
        ImGui::TableSetupColumn("Choroba");
        ImGui::TableSetupColumn("Akcje");
        ImGui::TableHeadersRow();
        
        const auto& db = frequency_loader.getFrequencyDatabase();
        const auto& results = search_triggered ? filtered_results : db;
        
        for (const auto& entry : results) {
            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            ImGui::Text("%u", entry.frequency_hz);
            
            ImGui::TableNextColumn();
            ImGui::Text("%s", entry.category.c_str());
            
            ImGui::TableNextColumn();
            ImGui::Text("%s", entry.disease_name.c_str());
            
            ImGui::TableNextColumn();
            char button_label[64];
            snprintf(button_label, sizeof(button_label), "Użyj##%u", entry.frequency_hz);
            if (ImGui::Button(button_label)) {
                onFrequencySelected(entry.frequency_hz);
            }
        }
        
        ImGui::EndTable();
    }
}

void GuiApp::renderStatusPanel() {
    ImGui::Text("Status systemu");
    ImGui::Separator();
    
    // Connection status
    ImGui::Text("Status połączenia: %s", status.connected ? "POŁĄCZONO" : "ROZŁĄCZONO");
    
    if (status.connected) {
        ImGui::SameLine();
        ImGui::Text("(%s:%d)", device_ip.c_str(), device_port);
        
        if (ImGui::Button("Rozłącz")) {
            disconnectFromDevice();
        }
    } else {
        static char ip_buffer[64] = "192.168.1.100";
        static int port_buffer = 5001;
        
        ImGui::InputText("IP urządzenia", ip_buffer, sizeof(ip_buffer));
        ImGui::SameLine();
        ImGui::InputInt("Port", &port_buffer);
        ImGui::SameLine();
        
        if (ImGui::Button("Połącz")) {
            connectToDevice(ip_buffer, port_buffer);
        }
    }
    
    ImGui::Separator();
    
    // System info
    ImGui::Text("Temperatura: %.1f°C", status.temperature);
    ImGui::Text("PWM: %s", status.pwm_running ? "URUCHOMIONY" : "ZATRZYMANI");
    ImGui::Text("Aktualna częstotliwość: %u Hz", status.current_frequency);
    ImGui::Text("Uptime: %u s", status.uptime_seconds);
    ImGui::Text("Wolna pamięć: %u B", status.free_memory);
    ImGui::Text("Stan bezpieczeństwa: %s", status.safety_state.c_str());
    
    ImGui::Separator();
    
    // Log panel
    ImGui::Text("Logi zdarzeń");
    ImGui::BeginChild("LogWindow", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);
    
    for (const auto& msg : log_messages) {
        ImGui::TextUnformatted(msg.c_str());
    }
    
    ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}

void GuiApp::renderUI() {
    ImGui::NewFrame();
    
    renderMenuBar();
    
    // Docking is available but we'll use simple tabs for now
    // ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    
    renderMainTabs();
    
    ImGui::Render();
}

void GuiApp::run() {
    running = true;
    
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
                running = false;
            }
        }
        
        // Update system status periodically
        if (status.connected) {
            std::string response = network.receiveData(10);
            if (!response.empty()) {
                // Parse response and update status
                // TODO: Implement proper status parsing
                addLog("Odebrano: " + response);
                
                // Update status fields from device response (placeholder for future implementation)
                // In a real implementation, parse the response to extract temperature, frequency, etc.
            }
            
            // Periodically update uptime counter
            static auto last_update = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_update).count() >= 1) {
                status.uptime_seconds++;
                last_update = now;
            }
        }
        
        // Render
        SDL_SetRenderDrawColor(renderer, (Uint8)(30), (Uint8)(30), (Uint8)(30), 255);
        SDL_RenderClear(renderer);
        
        renderUI();
        
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
        
        // Limit to ~60 FPS (16.67ms per frame)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void GuiApp::stop() {
    running = false;
}
