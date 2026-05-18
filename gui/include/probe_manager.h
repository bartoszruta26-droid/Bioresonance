/**
 * @file probe_manager.h
 * @brief Menadżer końcówek biorezonansowych
 */

#ifndef PROBE_MANAGER_H
#define PROBE_MANAGER_H

#include <map>
#include <vector>
#include <string>
#include "types.h"

class ProbeManager {
private:
    std::map<int, ProbeConfig> probes;
    ProbeMode current_mode;
    int active_channel;
    
public:
    ProbeManager();
    
    void initializeDefaultProbes();
    ProbeConfig* getProbe(int channel_id);
    std::vector<ProbeConfig*> getAllProbes();
    void setProbeMode(ProbeMode mode);
    ProbeMode getProbeMode() const;
    void enableProbe(int channel_id, bool enable);
    bool isProbeEnabled(int channel_id);
    std::string getProbeTypeName(ProbeType type);
    std::string getModeName(ProbeMode mode);
    
    // GUI-specific methods
    int getProbeCount() const { return probes.size(); }
    std::map<int, ProbeConfig>& getProbesMap() { return probes; }
};

#endif // PROBE_MANAGER_H
