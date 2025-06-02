#ifndef THREAD_INFO_HPP
#define THREAD_INFO_HPP

#include <pthread.h>
#include <papi.h>
#include <unistd.h>
#include <vector>

/**
 * @class ThreadInfo
 * @brief Manages PAPI performance counters for a specific thread.
 *
 * This class encapsulates PAPI event set creation, data collection, and delta/cumulative reporting
 * for an individual thread. It is instantiated and managed by `PapiManager` when a new thread is registered.
 *
 * Features:
 * - Initializes and attaches a PAPI event set to the thread.
 * - Stores previous counter values to compute deltas.
 * - Supports cumulative and interval-based reporting.
 *
 * @see PapiManager
 * @see LoggerManager
 * @see PapiEventRegistry
 */
class ThreadInfo {
public:
    /**
     * @brief Constructs a ThreadInfo instance and initializes its PAPI event set.
     * @param tid OS-level thread ID (e.g., from `syscall(SYS_gettid)`).
     * @param pthreadId POSIX thread ID (from `pthread_create`).
     *
     * This constructor initializes PAPI, assigns an event set, and attaches it to the specified thread.
     */
    ThreadInfo(pid_t tid, pthread_t pthreadId);

    /**
     * @brief Cleans up the PAPI event set and releases associated resources.
     */
    ~ThreadInfo();

    /**
     * @brief Checks if the thread has completed execution and been marked as finished.
     * @return `true` if finished, `false` otherwise.
     */
    bool isFinished() const;

    /**
     * @brief Marks the thread as finished, disabling further monitoring.
     */
    void markFinished();

    /**
     * @brief Updates stored counter values for this thread.
     *
     * Reads the current PAPI event values and updates internal state
     * without logging output. Used for internal state tracking.
     */
    void updateCounters();

    /**
     * @brief Logs the delta (change) of performance counters since the last update.
     * @param timestamp Timestamp string used for log tagging.
     * @param tag Logging tag identifying the thread type (e.g., "[MAIN]", "[THREAD]").
     */   
    void printDelta(const char* timestamp, const char* tag);

 
    /**
     * @brief Logs the cumulative (absolute) performance counter values.
     * @param timestamp Timestamp string used for log tagging.
     * @param tag Logging tag identifying the thread type (e.g., "[MAIN]", "[MONITOR]").
     */   
    void printCumulative(const char* timestamp, const char* tag);

    /**
     * @brief Returns the POSIX thread ID associated with this thread.
     * @return `pthread_t` value.
     */
    pthread_t getPthreadId() const;

    /**
     * @brief Returns the OS-level thread ID (`tid`) of this thread.
     * @return Thread ID (from `syscall(SYS_gettid)`).
     */   
    pid_t getTid() const;

private:
    pid_t tid_;                        ///< OS thread ID
    pthread_t pthreadId_;             ///< POSIX thread ID
    bool finished_;                   ///< Flag indicating if the thread has exited
    int eventSet_;                    ///< PAPI event set identifier
    std::vector<long long> prevValues_; ///< Last-read PAPI counter values (for delta computation)
};

#endif
