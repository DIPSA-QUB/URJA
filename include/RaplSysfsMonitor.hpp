#ifndef RAPL_SYSFS_MONITOR_HPP
#define RAPL_SYSFS_MONITOR_HPP

#include "IEnergyBackend.hpp"
#include "EnergyMonitor.hpp"
#include <string>
#include <vector>

/**
 * @class RaplSysfsMonitor
 * @brief Reads energy values from sysfs powercap interface.
 */
class RaplSysfsMonitor : public IRaplBackend {
public:
    void initialize() override;
    void monitor(const char* timestamp) override;
    size_t domainCount() const { return domains_.size(); }

private:
    struct Domain {
        std::string label;
        std::string path;
        long long last_value;
    };

    std::vector<Domain> domains_;
    long long readEnergy(const std::string& path);
    std::string formatLabel(const std::string& label, const std::string& path);
};

#endif
