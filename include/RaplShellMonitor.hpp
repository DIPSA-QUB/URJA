#ifndef RAPL_SHELL_MONITOR_HPP
#define RAPL_SHELL_MONITOR_HPP

#include "IEnergyBackend.hpp"
#include "EnergyMonitor.hpp"
#include <string>
#include <vector>

/**
 * @class RaplShellMonitor
 * @brief Backend implementation of IRaplBackend that uses an external shell script to read RAPL energy values.
 *
 * This class invokes an external script (typically `rapl_read.sh`) that reads energy data from the sysfs
 * RAPL interface using elevated permissions (via `sudo`). The output is parsed into structured domains,
 * and energy deltas are computed for logging purposes.
 *
 * Expected script format (line-based):
 *   name ; socket:domain_id ; domain_name ; energy_uj ; max_energy_uj
 *
 * This backend is useful when sysfs access is restricted or a simplified user-facing interface is preferred.
 * It requires:
 * - `URJA_ENERGY_BACKEND=shell`
 * - `SHELL_RAPL_CMD` path (`/var/shared/power/bin/rapl_read.sh` by default)
 * - Proper `sudoers` configuration to allow script execution without password prompts.
 */
class RaplShellMonitor : public IEnergyBackend {
public:
    /**
     * @brief Initializes the shell backend by capturing an initial energy snapshot.
     *
     * This function executes the RAPL shell command once and stores the first energy reading
     * for each domain to enable delta calculations during monitoring.
     */
    void initialize() override;

    /**
     * @brief Parses and logs energy deltas for each power domain using current and previous snapshots.
     * @param timestamp Timestamp string (typically in milliseconds).
     */
    void monitor(const char* timestamp) override;

private:
    struct ShellDomain {
        std::string index;              ///< Identifier string (e.g., "0:0", "0:1")
        std::string domain;             ///< Domain name (e.g., "package-0", "core", "dram")
        unsigned long long energy_uj;   ///< Current energy reading in microjoules
        unsigned long long max_energy_uj; ///< Maximum wraparound value for the energy counter
    };

    std::vector<ShellDomain> prev_; ///< Stores previous readings for delta computation

    /**
     * @brief Executes the shell script and parses its output into ShellDomain entries.
     * @return Vector of parsed ShellDomain values.
     */
    std::vector<ShellDomain> parseShellOutput();
};

#endif
