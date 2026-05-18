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
    const ProbeConfig* getProbe(int channel_id) const;
    std::vector<const ProbeConfig*> getAllProbes() const;
    void setProbeMode(ProbeMode mode);
    ProbeMode getProbeMode() const;
    bool enableProbe(int channel_id, bool enable);  // Returns true if successful
    bool isProbeEnabled(int channel_id) const;
    std::string getProbeTypeName(ProbeType type) const;
    std::string getModeName(ProbeMode mode) const;
    
    // GUI-specific methods
    int getProbeCount() const { return probes.size(); }
    const std::map<int, ProbeConfig>& getProbesMap() const { return probes; }
};

#endif // PROBE_MANAGER_H
