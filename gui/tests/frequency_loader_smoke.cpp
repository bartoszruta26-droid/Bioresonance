#include "frequency_loader.h"

#include <cstdio>
#include <fstream>
#include <string>

int main() {
    const std::string path = "frequency-loader-smoke.md";
    {
        std::ofstream out(path);
        if (!out) return 1;
        out << "100|Category A|Sub A|Disease Alpha / notes|NONE|38\n";
        out << "200|Category B|Sub B|Disease Beta|AM|40\n";
        out << "300|Category C|Sub C|   |FM|42\n";
    }

    FrequencyLoader loader;
    if (!loader.loadFromFile(path) || loader.getDatabaseSize() != 2) {
        std::remove(path.c_str());
        return 2;
    }
    if (loader.searchByDisease("dIsEaSe aLpHa").size() != 1) {
        std::remove(path.c_str());
        return 3;
    }
    if (loader.searchByFrequency(0, 150).size() != 1 ||
        loader.searchByFrequency(200, 0).size() != 1 ||
        loader.searchByFrequency(0, 0).size() != 0) {
        std::remove(path.c_str());
        return 4;
    }
    if (!loader.loadFromFile(path) || loader.getDatabaseSize() != 2) {
        std::remove(path.c_str());
        return 5;
    }

    {
        std::ofstream out(path, std::ios::trunc);
        out << "invalid|line\n";
    }
    if (loader.loadFromFile(path) || loader.getDatabaseSize() != 2) {
        std::remove(path.c_str());
        return 6;
    }

    std::remove(path.c_str());
    return 0;
}
