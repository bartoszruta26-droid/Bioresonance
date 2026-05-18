/**
 * @file probe_manager.cpp
 * @brief Implementacja menadżera końcówek biorezonansowych
 */

#include "probe_manager.h"

ProbeManager::ProbeManager() : current_mode(ProbeMode::SINGLE), active_channel(0) {
    initializeDefaultProbes();
}

void ProbeManager::initializeDefaultProbes() {
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

ProbeConfig* ProbeManager::getProbe(int channel_id) {
    auto it = probes.find(channel_id);
    return (it != probes.end()) ? &it->second : nullptr;
}

const ProbeConfig* ProbeManager::getProbe(int channel_id) const {
    auto it = probes.find(channel_id);
    return (it != probes.end()) ? &it->second : nullptr;
}

std::vector<const ProbeConfig*> ProbeManager::getAllProbes() const {
    std::vector<const ProbeConfig*> result;
    for (const auto& pair : probes) {
        if (pair.second.enabled) {
            result.push_back(&pair.second);
        }
    }
    return result;
}

void ProbeManager::setProbeMode(ProbeMode mode) {
    current_mode = mode;
}

ProbeMode ProbeManager::getProbeMode() const {
    return current_mode;
}

void ProbeManager::enableProbe(int channel_id, bool enable) {
    ProbeConfig* probe = getProbe(channel_id);
    if (probe) {
        probe->enabled = enable;
    }
}

bool ProbeManager::isProbeEnabled(int channel_id) const {
    const ProbeConfig* probe = getProbe(channel_id);
    return probe && probe->enabled;
}

std::string ProbeManager::getProbeTypeName(ProbeType type) const {
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

std::string ProbeManager::getModeName(ProbeMode mode) const {
    switch (mode) {
        case ProbeMode::SINGLE: return "Pojedyncza";
        case ProbeMode::DUAL_INDEPENDENT: return "Dual Niezależny";
        case ProbeMode::DUAL_SYNC: return "Dual Sync";
        case ProbeMode::MULTI_CHANNEL: return "Wielokanałowa";
        case ProbeMode::SEQUENTIAL: return "Sekwencyjna";
        default: return "Nieznany";
    }
}
