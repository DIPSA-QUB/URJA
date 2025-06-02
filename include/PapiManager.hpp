#ifndef PAPI_MANAGER_HPP
#define PAPI_MANAGER_HPP

#include <vector>
#include "ThreadInfo.hpp"

/**
 * @class PapiManager
 * @brief Singleton class that manages PAPI-based performance monitoring across threads.
 *
 * This class acts as the central coordinator for all PAPI event collection in URJA.
 * It tracks all active threads, assigns each a `ThreadInfo` object, and collects per-thread
 * hardware counter deltas using the PAPI library.
 *
 * Responsibilities:
 * - Initialize the global PAPI library.
 * - Register newly created threads (hooked via `pthread_create`).
 * - Periodically update counters for all registered threads.
 * - Distinguish between main, monitor, and worker threads for tagged logging.
 * - Print final cumulative statistics on program exit.
 */
class PapiManager {
public:
    /**
     * @brief Returns the singleton instance of PapiManager.
     * @return Reference to the global PapiManager.
     */
    static PapiManager& getInstance();

    /**
     * @brief Initializes the PAPI library and stores the main thread ID.
     *
     * This method should be called during URJA startup and before any thread monitoring begins.
     * It also performs version compatibility checks using `PAPI_library_init`.
     */
    void initialize();

    /**
     * @brief Sets the thread ID of the monitoring thread for tagging.
     * @param id POSIX thread ID of the monitoring thread.
     */
    void setMonitorThread(pthread_t id);

    /**
     * @brief Registers a new thread for monitoring and creates its PAPI event set.
     * @param tid Linux thread ID (e.g., from `syscall(SYS_gettid)`).
     * @param ptid POSIX thread ID (`pthread_t`) as returned by `pthread_create`.
     */ 
    void registerThread(pid_t tid, pthread_t ptid);

    /**
     * @brief Marks a thread as finished and disables further monitoring for it.
     * @param ptid POSIX thread ID of the thread that has completed.
     */
    void markThreadFinished(pthread_t ptid);

    /**
     * @brief Updates all active threads and logs their performance counter deltas.
     * @param timestamp Timestamp string to include in log entries.
     */
    void updateAllThreads(const char* timestamp);

    /**
     * @brief Logs cumulative PAPI statistics for all threads before shutdown.
     *
     * Typically called from `MonitorManager::stop()` or the destructor hook (`__attribute__((destructor))`).
     */
    void printFinalSummary();

private:
    /**
     * @brief Private constructor to enforce singleton pattern.
     */
    PapiManager();
    std::vector<ThreadInfo*> threads_;  ///< Container of registered threads being monitored.
    pthread_mutex_t mutex_;             ///< Mutex to protect concurrent access to the threads list.
};

#endif
