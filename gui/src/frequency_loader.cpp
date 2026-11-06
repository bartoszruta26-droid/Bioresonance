/**
 * @file frequency_loader.cpp
 * @brief Implementacja ładowania bazy częstotliwości
 */

#include "frequency_loader.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <limits>
#include <stdexcept>

FrequencyLoader::FrequencyLoader() {
}

bool FrequencyLoader::loadFromFile(const std::string& filepath) {
    std::ifstream freq_file(filepath);
    if (!freq_file.is_open()) {
        LOG_WARNING("Failed to open frequency file: " + filepath);
        return false;
    }
    
    std::vector<FrequencyEntry> parsed_entries;
    const auto trim = [](std::string value) {
        const size_t first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return std::string{};
        const size_t last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    };

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
                const std::string frequency_token = trim(tokens[0]);
                const std::string carrier_token = trim(tokens[5]);
                size_t frequency_chars = 0;
                size_t carrier_chars = 0;
                const unsigned long parsed_frequency = std::stoul(frequency_token, &frequency_chars);
                const unsigned long parsed_carrier = std::stoul(carrier_token, &carrier_chars);
                if (trim(frequency_token.substr(frequency_chars)).size() != 0 ||
                    trim(carrier_token.substr(carrier_chars)).size() != 0 ||
                    parsed_frequency > std::numeric_limits<uint32_t>::max() ||
                    parsed_carrier > std::numeric_limits<uint32_t>::max()) {
                    throw std::out_of_range("numeric field out of range or contains trailing data");
                }

                FrequencyEntry entry;
                entry.frequency_hz = static_cast<uint32_t>(parsed_frequency);
                entry.category = trim(tokens[1]);
                entry.subcategory = trim(tokens[2]);
                entry.description = trim(tokens[3]);
                entry.modulation = trim(tokens[4]);
                entry.carrier_khz = static_cast<uint32_t>(parsed_carrier);
                if (entry.frequency_hz == 0 || entry.category.empty() || entry.description.empty()) {
                    throw std::invalid_argument("required frequency field is empty or zero");
                }

                // Extract disease name (before / separator)
                size_t slash_pos = entry.description.find('/');
                if (slash_pos != std::string::npos) {
                    entry.disease_name = trim(entry.description.substr(0, slash_pos));
                } else {
                    entry.disease_name = trim(entry.description);
                }
                
                parsed_entries.push_back(entry);
            } catch (const std::exception& e) {
                // Log parsing errors for debugging
                LOG_DEBUG("Skipping invalid frequency line: " + line + " - Error: " + e.what());
                continue;
            }
        }
    }
    
    freq_file.close();
    
    if (parsed_entries.empty()) {
        LOG_WARNING("No valid frequency entries found in: " + filepath);
        return false;
    }

    // Replace atomically at the container level; repeated loads do not duplicate rows.
    frequency_database.swap(parsed_entries);
    LOG_INFO("Loaded " + std::to_string(frequency_database.size()) + " frequencies from: " + filepath);
    return true;
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
    const auto lowercase = [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); };
    std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), lowercase);
    
    try {
        for (const auto& entry : frequency_database) {
            std::string desc_lower = entry.description;
            std::transform(desc_lower.begin(), desc_lower.end(), desc_lower.begin(), lowercase);
            
            std::string disease_lower = entry.disease_name;
            std::transform(disease_lower.begin(), disease_lower.end(), disease_lower.begin(), lowercase);
            
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
        const uint64_t lower = (freq_hz > tolerance)
            ? static_cast<uint64_t>(freq_hz - tolerance) : 0;
        const uint64_t upper = static_cast<uint64_t>(freq_hz) + tolerance;
        if (static_cast<uint64_t>(entry.frequency_hz) >= lower &&
            static_cast<uint64_t>(entry.frequency_hz) <= upper) {
            result.push_back(entry);
        }
    }
    return result;
}
