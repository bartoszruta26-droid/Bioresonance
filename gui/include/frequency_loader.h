/**
 * @file frequency_loader.h
 * @brief Ładowanie bazy częstotliwości z pliku frequencies.md
 */

#ifndef FREQUENCY_LOADER_H
#define FREQUENCY_LOADER_H

#include <vector>
#include <string>
#include "types.h"

class FrequencyLoader {
private:
    std::vector<FrequencyEntry> frequency_database;
    
public:
    FrequencyLoader();
    
    bool loadFromFile(const std::string& filepath);
    const std::vector<FrequencyEntry>& getFrequencyDatabase() const { return frequency_database; }
    int getDatabaseSize() const { return frequency_database.size(); }
    
    // Search methods
    std::vector<FrequencyEntry> searchByCategory(const std::string& category);
    std::vector<FrequencyEntry> searchByDisease(const std::string& disease);
    std::vector<FrequencyEntry> searchByFrequency(uint32_t freq_hz, uint32_t tolerance = 100);
};

#endif // FREQUENCY_LOADER_H
