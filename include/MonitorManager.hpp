#ifndef MONITOR_MANAGER_HPP
#define MONITOR_MANAGER_HPP

/**
 * @class MonitorManager
 * @brief Manages the background monitoring thread responsible for logging energy and performance data.
 *
 * This class spawns and manages a dedicated thread that periodically samples:
 * - Energy consumption using the `EnergyMonitor` facade (RAPL)
 * - Per-thread hardware performance counters using `PapiManager`
 *
 * The sampling interval is configured using the `URJA_INTERVAL_MS` environment variable.
 * This class is initialized automatically via the `init_urja()` constructor hook.
 *
 * Usage:
 * - `MonitorManager::start()` — creates and detaches the monitoring thread.
 * - `MonitorManager::stop()` — stops the thread and prints the final cumulative summary.
 */
class MonitorManager {
public:
    /**
     * @brief Starts the monitoring thread in detached mode.
     *
     * This method spawns a background thread that continuously logs energy and performance
     * statistics at the configured interval. It is typically invoked once during initialization.
     */
    static void start();

    /**
     * @brief Stops the monitoring thread and flushes final performance data.
     *
     * This method sets the control flag to terminate the loop and calls
     * `PapiManager::printFinalSummary()` to log final statistics for all threads.
     */
    static void stop();

private:
    /**
     * @brief The main loop executed by the monitoring thread.
     *
     * This method is called by `pthread_create()` and repeatedly logs:
     * - RAPL energy deltas via `EnergyMonitor`
     * - PAPI counter deltas via `PapiManager`
     *
     * The loop sleeps between iterations for `URJA_INTERVAL_MS` milliseconds.
     *
     * @param arg Unused thread argument.
     * @return nullptr on exit.
     */
    static void* monitorLoop(void* arg);
};

#endif
