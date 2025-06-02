#include "EnergyMonitor.hpp"
#include "RaplSysfsMonitor.hpp"
#include "RaplShellMonitor.hpp"
#include "LoggerManager.hpp"
#include <cstdio>
#include <cstring>


/**
 * @brief Constructs the EnergyMonitor singleton with no active backend.
 */
EnergyMonitor::EnergyMonitor() : backend_(nullptr) {}

/**
 * @brief Retrieves the singleton instance of the EnergyMonitor.
 * 
 * @return Reference to the globally accessible EnergyMonitor instance.
 */
EnergyMonitor& EnergyMonitor::getInstance() {
    static EnergyMonitor instance;
    return instance;
}

/**
 * @brief Initializes the appropriate energy monitoring backend.
 *
 * The backend is selected based on the environment variable `URJA_ENERGY_BACKEND`.
 * Supported values:
 * - `"shell"` → Uses RaplShellMonitor (executes external script)
 * - `"sysfs"` → Uses RaplSysfsMonitor (reads directly from `/sys/class/powercap`)
 *
 * Logs are generated indicating which backend was selected or if initialization failed.
 */
void EnergyMonitor::initialize() {
    const char* backend = std::getenv("URJA_ENERGY_BACKEND");

    // Option 1: Use shell-based backend (external script)
    if (backend && std::strcmp(backend, "shell") == 0) {
        auto* shell = new RaplShellMonitor();
        shell->initialize();
        backend_ = shell;
        LoggerManager::getInstance().logLine("INIT","DEBUG", "Using RAPL shell backend.");
        return;
    }

    // Option 2: Use sysfs-based backend (preferred for efficiency and accuracy)
    else if (backend && std::strcmp(backend, "sysfs") == 0) {
        auto* sysfs = new RaplSysfsMonitor();
        sysfs->initialize();
        
        // Only use sysfs backend if domains are successfully discovered
        if (sysfs->domainCount() > 0) {
            backend_ = sysfs;
            LoggerManager::getInstance().logLine("INIT","DEBUG", "Using RAPL sysfs backend.");
            return;
        }

        // Log error only if sysfs was explicitly requested but failed
        else {
            delete sysfs;
            if (backend && std::strcmp(backend, "sysfs") == 0) {
                LoggerManager::getInstance().logLine("INIT","ERROR", "Sysfs backend forced, but no RAPL domains found.");
                return;
            }
        }
    }
    // No valid backend specified or available
    LoggerManager::getInstance().logLine("INIT","ERROR", "No energy backend specified.");
}

/**
 * @brief Invokes the current backend to log energy data at the given timestamp.
 *
 * @param timestamp A string representing the current timestamp in milliseconds.
 */
void EnergyMonitor::monitor(const char* timestamp) {
    if (backend_)
        backend_->monitor(timestamp);
    else
        LoggerManager::getInstance().logLine("INIT","ERROR", "Energy monitor not initialized.");
}