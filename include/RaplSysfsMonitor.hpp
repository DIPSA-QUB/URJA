#ifndef RAPL_SYSFS_MONITOR_HPP
#define RAPL_SYSFS_MONITOR_HPP

#include "IEnergyBackend.hpp"
#include <string>
#include <vector>

class RaplSysfsMonitor : public IEnergyBackend {
public:
    RaplSysfsMonitor() = default;
    ~RaplSysfsMonitor(); // Added destructor for cleanup

    void initialize() override;
    void monitor(const char* timestamp) override;
    size_t domainCount() const { return domains_.size(); }

private:
    struct Domain {
        std::string label;
        int fd;             // Store the open file descriptor
        uint64_t last_value;
        uint64_t max_value;
    };

    std::vector<Domain> domains_;
    
    // Helper to read from an open FD
    uint64_t readEnergy(int fd);
    std::string formatLabel(const std::string& label, const std::string& path);
};

#endif