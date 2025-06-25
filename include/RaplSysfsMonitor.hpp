#ifndef RAPL_SYSFS_MONITOR_HPP
#define RAPL_SYSFS_MONITOR_HPP

#include "IEnergyBackend.hpp"
#include "EnergyMonitor.hpp"
#include <string>
#include <vector>

class RaplSysfsMonitor : public IEnergyBackend {
public:
    void initialize() override;
    void monitor(const char* timestamp) override;
    size_t domainCount() const { return domains_.size(); }

private:
    struct Domain {
        std::string label;
        std::string path;
        uint64_t last_value;
        uint64_t max_value;
    };

    std::vector<Domain> domains_;
    uint64_t readEnergy(const std::string& path);
    std::string formatLabel(const std::string& label, const std::string& path);
};

#endif
