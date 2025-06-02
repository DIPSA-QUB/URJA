#include "MonitorManager.hpp"
#include "PapiManager.hpp"
#include "EnergyMonitor.hpp"
#include "LoggerManager.hpp"
#include <unistd.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>



// ----------------------------------------------------------------------------
// Internal State
// ----------------------------------------------------------------------------

/// Sleep interval between monitoring cycles (in microseconds)
static useconds_t monitorSleepUs = 0;

/// Control flag for the monitoring loop
static volatile int keepRunning = 1;

/// Thread handle for the monitoring thread
static pthread_t monitorThread;

/**
 * @brief Initializes the monitor sleep interval from the environment variable.
 *
 * Environment variable:
 * - `URJA_INTERVAL_MS` — sets the delay between monitoring samples in milliseconds.
 *
 * Logs an error if the value is missing or invalid.
 */
static void initSleepDuration() {
    const char* env = std::getenv("URJA_INTERVAL_MS");
    if (env) {
        int msec = std::atoi(env);
        if (msec > 0) {
            monitorSleepUs = msec * 1000;
            LoggerManager::getInstance().logLine("INIT","DEBUG", "Monitoring interval set to " + std::to_string(msec) +" ms.");
        } else {
            LoggerManager::getInstance().logLine("INIT","ERROR", std::string("Invalid URJA_INTERVAL_MS: ") + env);
        }
    }
}

/**
 * @brief The main loop executed by the monitoring thread.
 *
 * Performs the following each cycle:
 * - Timestamps the sample
 * - Calls `EnergyMonitor::monitor()` to log energy deltas
 * - Calls `PapiManager::updateAllThreads()` to log hardware counters
 * - Logs the duration of the monitoring cycle
 *
 * Loop terminates when `keepRunning` is set to 0.
 *
 * @param unused Not used (standard pthread signature)
 * @return nullptr on thread exit
 */
void* MonitorManager::monitorLoop(void*) {
    static bool initialized = false;
    if (!initialized) {
	initSleepDuration();
        EnergyMonitor::getInstance().initialize();
        initialized = true;
    }	
    
    while (keepRunning) {
	    struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        // Format timestamp as milliseconds since monotonic start
        char timestamp[64];
        snprintf(timestamp, sizeof(timestamp), "%lld", (long long)start.tv_sec * 1000 + start.tv_nsec / 1000000);

        // Run energy and performance monitoring
        EnergyMonitor::getInstance().monitor(timestamp);
        PapiManager::getInstance().updateAllThreads(timestamp);

        // Measure and log the monitoring loop duration
	    clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                            (end.tv_nsec - start.tv_nsec) / 1e6;

        LoggerManager::getInstance().logLine(timestamp, "DEBUG", "Monitoring loop duration: " + std::to_string(elapsed_ms) + " ms");
        usleep(monitorSleepUs); // Wait before next sample
    }
    return nullptr;
}

/**
 * @brief Starts the monitoring thread.
 *
 * This function creates and detaches the background thread that periodically monitors
 * energy and performance counters. It also notifies the `PapiManager` of the monitor thread ID.
 */
void MonitorManager::start() {
    pthread_create(&monitorThread, nullptr, monitorLoop, nullptr);
    pthread_detach(monitorThread);
    PapiManager::getInstance().setMonitorThread(monitorThread);
}

/**
 * @brief Stops the monitoring loop and prints the final cumulative summary.
 *
 * This function is typically called during program shutdown (from the destructor hook).
 */
void MonitorManager::stop() {
    keepRunning = 0;
    usleep(monitorSleepUs);
    PapiManager::getInstance().printFinalSummary();
}
