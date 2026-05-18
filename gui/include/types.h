/**
 * @file types.h
 * @brief Wspólne typy danych dla aplikacji GUI
 */

#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>
#include <cstdint>

// Typy końcówek biorezonansowych
enum class ProbeType {
    FLAT_COIL,           // Cewka płaska
    FERRITE_ROD,         // Cewka ferrytowa
    CAPACITIVE_PLATE,    // Płyta kapacytacyjna
    PEN_APPLICATOR,      // Aplikator punktowy
    MAT_APPLICATOR,      // Mata EMF
    LOCAL_PAD,           // Podkładka lokalna
    RING_APPLICATOR,     // Pierścień
    COMB_MULTI,          // Kombinacja wielokanałowa
    CUSTOM               // Konfiguracja niestandardowa
};

// Tryb pracy końcówki
enum class ProbeMode {
    SINGLE,              // Pojedyncza końcówka
    DUAL_INDEPENDENT,    // Dwie niezależne końcówki
    DUAL_SYNC,           // Dwie zsynchronizowane
    MULTI_CHANNEL,       // Wielokanałowa (4+)
    SEQUENTIAL           // Sekwencyjna rotacja
};

// Typ modulacji
enum class ModulationType {
    NONE = 0,
    AM = 1,              // Modulacja amplitudy
    FM = 2,              // Modulacja częstotliwości
    BURST = 3,           // Impulsowa
    SWEEP = 4            // Przemiatający
};

// Konfiguracja pojedynczej końcówki
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

// Pakiet terapeutyczny wysyłany do Arduino
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

// Stan systemu
struct SystemStatus {
    bool connected;
    float temperature;
    bool pwm_running;
    uint32_t current_frequency;
    uint32_t uptime_seconds;
    uint16_t free_memory;
    std::string safety_state;
    std::vector<ProbeConfig> active_probes;
    
    SystemStatus() : 
        connected(false),
        temperature(0.0f),
        pwm_running(false),
        current_frequency(0),
        uptime_seconds(0),
        free_memory(0),
        safety_state("Nieznany") {}
};

// Wpis z bazy częstotliwości
struct FrequencyEntry {
    uint32_t frequency_hz;
    std::string category;
    std::string subcategory;
    std::string description;
    std::string disease_name;
    std::string modulation;
    uint32_t carrier_khz;
};

#endif // TYPES_H
