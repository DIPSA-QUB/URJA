#ifndef ENERGY_MONITOR_HPP
#define ENERGY_MONITOR_HPP

#include "IEnergyBackend.hpp"
#include <memory>

class EnergyMonitor {
public:
    static EnergyMonitor& getInstance();
    void initialize();
    void monitor(const char* timestamp);

private:
    EnergyMonitor();
    std::unique_ptr<IEnergyBackend> backend_;
};

#endif
