#ifndef ENERGY_MONITOR_HPP
#define ENERGY_MONITOR_HPP

/**
 * @class EnergyMonitor
 * @brief Singleton class that provides a unified interface for energy consumption monitoring using RAPL or APM.
 *
 * This class abstracts away the details of the backend implementation and chooses either the sysfs-based or 
 * shell-based energy monitoring mechanism based on the environment variable `URJA_ENERGY_BACKEND`.
 *
 * - If `URJA_ENERGY_BACKEND=sysfs`, it uses the sysfs interface under `/sys/class/powercap/intel-rapl`.
 * - If `URJA_ENERGY_BACKEND=shell`, it invokes a wrapper script (e.g., `rapl_read.sh`) for energy readings.
 *
 * Usage assumes:
 * - Initialization via `initialize()` must be called once before monitoring.
 * - Periodic invocation of `monitor()` with timestamps to log energy deltas.
 *
 * This class is used internally by `MonitorManager` during periodic sampling.
 */
class EnergyMonitor {
public:
    /**
     * @brief Returns the singleton instance of the EnergyMonitor.
     * @return Reference to the EnergyMonitor instance.
     */
    static EnergyMonitor& getInstance();

    /**
     * @brief Initializes the energy backend (sysfs or shell) based on environment variables.
     *
     * If the backend is unavailable or invalid, logs an error and disables energy monitoring.
     * Expected environment variable: `URJA_ENERGY_BACKEND`.
     */
    void initialize();

    /**
     * @brief Invokes the backend monitor to record energy usage at a given timestamp.
     * @param timestamp The current timestamp in milliseconds as a string.
     *
     * This method is periodically called in the monitoring loop to log energy usage deltas.
     */
    void monitor(const char* timestamp);

private:
    /**
     * @brief Private constructor to enforce singleton pattern.
     */
    EnergyMonitor();

    /**
     * @brief Pointer to the active RAPL backend implementation (sysfs or shell).
     */
    class IEnergyBackend* backend_;
};

#endif
