#ifndef RAPL_SHELL_MONITOR_HPP
#define RAPL_SHELL_MONITOR_HPP

#include "IEnergyBackend.hpp"
#include <string>
#include <vector>
#include <cstdint>

class RaplShellMonitor : public IEnergyBackend {
public:
    void initialize() override;
    void monitor(const char* timestamp) override;

private:
    struct Domain {
        std::string label;
        uint64_t maxValue;
        uint64_t prevValue;
    };

    std::vector<Domain> domains_;

    std::vector<std::string> runCommandLines(const std::string& cmd);
};

#endif
