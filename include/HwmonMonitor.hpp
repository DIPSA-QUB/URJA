#ifndef HWMON_MONITOR_HPP
#define HWMON_MONITOR_HPP

#include "IEnergyBackend.hpp"
#include <string>
#include <vector>

class HwmonMonitor : public IEnergyBackend {
public:
    void initialize() override;
    void monitor(const char* timestamp) override;
    size_t domainCount() const { return domains_.size(); }

private:
    struct Domain {
        std::string label;
        std::string path;
    };

    std::vector<Domain> domains_;
    float readTemperature(const std::string& path);
    std::string formatLabel(const std::string& sensor, const std::string& label, const std::string& input);
};

#endif
