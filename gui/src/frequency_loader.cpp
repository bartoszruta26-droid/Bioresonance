/**
 * @file frequency_loader.cpp
 * @brief Implementacja ładowania bazy częstotliwości
 */

#include "frequency_loader.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>

FrequencyLoader::FrequencyLoader() {
}

bool FrequencyLoader::loadFromFile(const std::string& filepath) {
    std::ifstream freq_file(filepath);
    if (!freq_file.is_open()) {
        LOG_WARNING("Failed to open frequency file: " + filepath);
        return false;
    }
    
    std::string line;
    while (std::getline(freq_file, line)) {
        // Skip comments, headers, empty lines
        if (line.empty() || line[0] == '#' || line[0] == '-' || line[0] == '=') continue;
        if (line.find("FREQUENCY_DATA") != std::string::npos) continue;
        if (line.find("freq_hz|") != std::string::npos) continue;
        
        // Parse frequency data: freq_hz|category|subcategory|description|modulation|carrier_khz
        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(iss, token, '|')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 6) {
            try {
                uint32_t freq = std::stoul(tokens[0]);
                
                FrequencyEntry entry;
                entry.frequency_hz = freq;
                entry.category = tokens[1];
                entry.subcategory = tokens[2];
                entry.description = tokens[3];
                entry.modulation = tokens[4];
                entry.carrier_khz = std::stoul(tokens[5]);
                
                // Extract disease name (before / separator)
                size_t slash_pos = entry.description.find('/');
                if (slash_pos != std::string::npos) {
                    entry.disease_name = entry.description.substr(0, slash_pos);
                    // Trim whitespace
                    entry.disease_name.erase(0, entry.disease_name.find_first_not_of(" \t"));
                    entry.disease_name.erase(entry.disease_name.find_last_not_of(" \t") + 1);
                } else {
                    entry.disease_name = entry.description;
                }
                
                frequency_database.push_back(entry);
            } catch (const std::exception& e) {
                // Log parsing errors for debugging
                LOG_DEBUG("Skipping invalid frequency line: " + line + " - Error: " + e.what());
                continue;
            }
        }
    }
    
    freq_file.close();
    
    if (frequency_database.empty()) {
        LOG_WARNING("No valid frequency entries found in: " + filepath);
    } else {
        LOG_INFO("Loaded " + std::to_string(frequency_database.size()) + " frequencies from: " + filepath);
    }
    
    return !frequency_database.empty();
}

std::vector<FrequencyEntry> FrequencyLoader::searchByCategory(const std::string& category) {
    std::vector<FrequencyEntry> result;
    for (const auto& entry : frequency_database) {
        if (entry.category.find(category) != std::string::npos || 
            entry.subcategory.find(category) != std::string::npos) {
            result.push_back(entry);
        }
    }
    return result;
}

std::vector<FrequencyEntry> FrequencyLoader::searchByDisease(const std::string& disease) {
    std::vector<FrequencyEntry> result;
    std::string search_lower = disease;
    std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);
    
    try {
        for (const auto& entry : frequency_database) {
            std::string desc_lower = entry.description;
            std::transform(desc_lower.begin(), desc_lower.end(), desc_lower.begin(), ::tolower);
            
            std::string disease_lower = entry.disease_name;
            std::transform(disease_lower.begin(), disease_lower.end(), disease_lower.begin(), ::tolower);
            
            if (desc_lower.find(search_lower) != std::string::npos || 
                disease_lower.find(search_lower) != std::string::npos) {
                result.push_back(entry);
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Błąd podczas wyszukiwania chorób: ") + e.what());
    }
    return result;
}

std::vector<FrequencyEntry> FrequencyLoader::searchByFrequency(uint32_t freq_hz, uint32_t tolerance) {
    std::vector<FrequencyEntry> result;
    for (const auto& entry : frequency_database) {
        if (entry.frequency_hz >= freq_hz - tolerance && 
            entry.frequency_hz <= freq_hz + tolerance) {
            result.push_back(entry);
        }
    }
    return result;
}
