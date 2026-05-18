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
#ifdef _WIN32
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", localtime(&time));
#else
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", localtime(&time));
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
}

void GuiApp::onProbeEnabledChanged(int probe_id, bool enabled) {
    probe_manager.enableProbe(probe_id, enabled);
    addLog("Końcówka " + std::to_string(probe_id) + (enabled ? " włączona" : " wyłączona"));
}

void GuiApp::onFrequencySelected(uint32_t frequency_hz) {
    ProbeConfig* probe = probe_manager.getProbe(selected_probe);
    if (probe) {
        probe->frequency_hz_x100 = frequency_hz * 100;
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
        packet.checksum = 0;  // TODO: Calculate proper checksum
        
        if (network.sendTherapyPacket(packet)) {
            status.pwm_running = true;
            addLog("Rozpoczęto terapię na kanale " + std::to_string(probe->channel_id));
        } else {
            addLog("Błąd wysyłania pakietu terapeutycznego");
        }
    }
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
    
    auto& probes = probe_manager.getProbesMap();
    for (auto& [id, config] : probes) {
        bool is_selected = (selected_probe == id);
        
        if (ImGui::Selectable(config.name.c_str(), is_selected)) {
            onProbeSelected(id);
        }
        
        // Show enabled checkbox
        char checkbox_label[64];
        snprintf(checkbox_label, sizeof(checkbox_label), "##enable_%d", id);
        if (ImGui::Checkbox(checkbox_label, &config.enabled)) {
            onProbeEnabledChanged(id, config.enabled);
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
        ImGui::InputText("Nazwa", &probe->name[0], probe->name.size());
        
        ImGui::SliderFloat("Częstotliwość (Hz)", 
                          reinterpret_cast<float*>(&probe->frequency_hz_x100), 
                          100.0f, 1000000.0f);
        
        ImGui::SliderInt("Czas trwania (s)", (int*)&probe->duration_sec, 1, 3600);
        
        ImGui::SliderInt("Wypełnienie PWM (%)", (int*)&probe->duty_cycle, 0, 100);
        
        ImGui::SliderInt("Intensywność", (int*)&probe->intensity_level, 0, 4095);
        
        // Modulation type combo
        const char* modulation_types[] = {"Brak", "AM", "FM", "BURST", "SWEEP"};
        int current_mod = (int)probe->modulation;
        if (ImGui::Combo("Modulacja", &current_mod, modulation_types, 5)) {
            probe->modulation = (ModulationType)current_mod;
        }
        
        if (probe->modulation != ModulationType::NONE) {
            ImGui::SliderInt("Częst. modulacji (Hz)", (int*)&probe->modulation_freq_hz, 1, 100);
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
    static std::vector<FrequencyEntry> filtered_results;
    static bool search_triggered = false;
    
    ImGui::Text("Przeglądarka częstotliwości");
    ImGui::Separator();
    
    // Search bar
    ImGui::InputText("Szukaj", search_buffer, sizeof(search_buffer));
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
            }
        }
        
        // Render
        SDL_SetRenderDrawColor(renderer, (Uint8)(30), (Uint8)(30), (Uint8)(30), 255);
        SDL_RenderClear(renderer);
        
        renderUI();
        
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
        
        // Limit to 60 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void GuiApp::stop() {
    running = false;
}
