#ifndef ENERGY_MONITOR_HPP
#define ENERGY_MONITOR_HPP

/**
 * @class EnergyMonitor
 * @brief Facade that delegates RAPL monitoring to sysfs or shell-based backend.
 */
class EnergyMonitor {
public:
    static EnergyMonitor& getInstance();
    void initialize();
    void monitor(const char* timestamp);

private:
    EnergyMonitor();
    class IRaplBackend* backend_; ///< Pointer to concrete implementation
};

#endif
