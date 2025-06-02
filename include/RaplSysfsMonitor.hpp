#ifndef RAPL_SYSFS_MONITOR_HPP
#define RAPL_SYSFS_MONITOR_HPP

#include "IEnergyBackend.hpp"
#include "EnergyMonitor.hpp"
#include <string>
#include <vector>

/**
 * @class RaplSysfsMonitor
 * @brief Backend implementation of IRaplBackend that reads energy values from the Linux sysfs powercap interface.
 *
 * This class interacts directly with the `/sys/class/powercap/intel-rapl` filesystem to monitor
 * energy usage across hardware domains such as package, core, DRAM, and uncore. It scans available
 * RAPL domains during initialization and periodically computes energy deltas.
 *
 * This backend is typically preferred for performance and accuracy if direct access to sysfs is available.
 *
 * Requirements:
 * - `URJA_ENERGY_BACKEND=sysfs`
 * - Read access to `/sys/class/powercap/intel-rapl/`
 *
 * Each RAPL domain discovered is mapped to a label and tracked during monitoring.
 */
class RaplSysfsMonitor : public IEnergyBackend {
public:
    /**
     * @brief Initializes the RAPL sysfs backend by discovering available energy domains.
     *
     * This involves globbing the RAPL directory for valid energy files, reading their
     * current energy values, and creating labeled domain entries.
     */
    void initialize() override;

    /**
     * @brief Reads and logs energy deltas for each discovered RAPL domain.
     * @param timestamp Current timestamp in milliseconds (string format).
     */ 
    void monitor(const char* timestamp) override;

    /**
     * @brief Returns the number of valid energy domains detected.
     * @return Number of RAPL domains being monitored.
     */  
    size_t domainCount() const { return domains_.size(); }

private:
    /**
     * @struct Domain
     * @brief Represents a power domain with its sysfs path, label, and last-read energy value.
     */
    struct Domain {
        std::string label;
        std::string path;
        long long last_value;
    };

    /**
     * @brief Reads the current energy value from a sysfs file.
     * @param path Path to the energy_uj file.
     * @return Energy value in microjoules, or -1 on error.
     */
    std::vector<Domain> domains_;

     /**
     * @brief Reads the current energy value from a sysfs file.
     * @param path Path to the energy_uj file.
     * @return Energy value in microjoules, or -1 on error.
     */   
    long long readEnergy(const std::string& path);

    /**
     * @brief Constructs a standardized label for a domain based on its sysfs path and name.
     * @param label Raw domain name read from the sysfs `name` file.
     * @param path Full sysfs path to the domain’s energy_uj file.
     * @return Formatted label in uppercase and domain-index format.
     */
    std::string formatLabel(const std::string& label, const std::string& path);
};

#endif
