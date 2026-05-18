/**
 * @file gui_app.h
 * @brief Główna klasa aplikacji GUI przy użyciu Dear ImGui i SDL2
 */

#ifndef GUI_APP_H
#define GUI_APP_H

#include <string>
#include <vector>
#include <memory>
#include "types.h"
#include "network_client.h"
#include "probe_manager.h"
#include "frequency_loader.h"

// Forward declarations for SDL and ImGui
struct SDL_Window;
struct SDL_Renderer;

class GuiApp {
private:
    // SDL2 components
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    // Application components
    NetworkClient network;
    ProbeManager probe_manager;
    FrequencyLoader frequency_loader;
    
    // Application state
    SystemStatus status;
    std::vector<std::string> log_messages;
    int selected_probe;
    bool running;
    
    std::string device_ip;
    int device_port;
    
    // UI state
    bool show_frequency_browser;
    bool show_probe_settings;
    bool show_system_status;
    int current_tab;
    
    // Methods
    bool initSDL();
    void cleanupSDL();
    void renderUI();
    void renderMenuBar();
    void renderMainTabs();
    void renderProbePanel();
    void renderFrequencyBrowser();
    void renderStatusPanel();
    void renderLogPanel();
    void addLog(const std::string& message);
    
public:
    GuiApp();
    ~GuiApp();
    
    bool init();
    void run();
    void stop();
    
    bool connectToDevice(const std::string& ip, int port);
    void disconnectFromDevice();
    bool isConnected() const { return network.isConnected(); }
    
    // Callbacks
    void onProbeSelected(int probe_id);
    void onProbeEnabledChanged(int probe_id, bool enabled);
    void onFrequencySelected(uint32_t frequency_hz);
    void onStartTherapy();
    void onStopTherapy();
};

#endif // GUI_APP_H
