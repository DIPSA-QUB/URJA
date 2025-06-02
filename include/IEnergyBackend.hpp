#ifndef IENERGY_BACKEND_HPP
#define IENERGY_BACKEND_HPP

/**
 * @class IEnergyBackend
 * @brief Abstract interface for Energy monitoring backends.
 *
 * This interface is implemented by concrete classes that read energy consumption data
 * from different sources, such as:
 * - `RaplSysfsMonitor`: Uses the Linux sysfs `/sys/class/powercap/intel-rapl/` interface.
 * - `RaplShellMonitor`: Invokes an external shell script (e.g., `rapl_read.sh`) to fetch RAPL values.
 *
 * The EnergyMonitor class uses this interface to abstract over the backend choice.
 */
class IEnergyBackend {
public:
    /**
     * @brief Virtual destructor for proper cleanup in derived classes.
     */
    virtual ~IEnergyBackend() = default;

    /**
     * @brief Initializes the backend by detecting domains or preparing resources.
     *
     * This method should be called once before calling `monitor()`.
     * It may perform file discovery, open handles, or execute preparation scripts depending on implementation.
     */
    virtual void initialize() = 0;

    /**
     * @brief Reads and logs the energy usage for the current monitoring interval.
     * @param timestamp The current timestamp in milliseconds (string format) for consistent logging.
     *
     * The implementation should compute energy deltas and output a structured log line for each power domain.
     */
    virtual void monitor(const char* timestamp) = 0;
};

#endif