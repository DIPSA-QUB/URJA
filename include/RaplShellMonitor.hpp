#ifndef RAPL_SHELL_MONITOR_HPP
#define RAPL_SHELL_MONITOR_HPP

#include "IEnergyBackend.hpp"
#include "EnergyMonitor.hpp"
#include <string>
#include <vector>

/**
 * @class RaplShellMonitor
 * @brief Uses external script to read energy values.
 */
class RaplShellMonitor : public IRaplBackend {
public:
    void initialize() override;
    void monitor(const char* timestamp) override;

private:
    struct ShellDomain {
        std::string index;
        std::string domain;
        unsigned long long energy_uj;
        unsigned long long max_energy_uj;
    };

    std::vector<ShellDomain> prev_;
    std::vector<ShellDomain> parseShellOutput();
};

#endif
