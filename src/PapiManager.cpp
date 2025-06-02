#include "PapiManager.hpp"
#include "LoggerManager.hpp"
#include <algorithm>
#include <cstdio>

/// Thread ID of the monitoring thread (set by MonitorManager)
static pthread_t monitorThreadId_;

/// Thread ID of the main application thread (captured at initialization)
static pthread_t mainThreadId_;

/**
 * @brief Constructs the PapiManager and initializes its internal mutex.
 */
PapiManager::PapiManager() {
    pthread_mutex_init(&mutex_, nullptr);
}

/**
 * @brief Retrieves the singleton instance of the PapiManager.
 * @return Reference to the singleton instance
 */
PapiManager& PapiManager::getInstance() {
    static PapiManager instance;
    return instance;
}

/**
 * @brief Sets the ID of the monitoring thread.
 * @param id POSIX thread ID of the monitor
 */
void PapiManager::setMonitorThread(pthread_t id) {
    monitorThreadId_ = id;
}

/**
 * @brief Initializes the PAPI library and records the main thread ID.
 *
 * This should be called once during program startup. Logs an error if
 * the PAPI version check fails.
 */
void PapiManager::initialize() {
    mainThreadId_ = pthread_self();
    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT)
        LoggerManager::getInstance().logLine("INIT","ERROR", "PAPI initialization failed.");
}

/**
 * @brief Registers a thread for performance monitoring.
 *
 * Prevents duplicate registration based on TID.
 *
 * @param tid OS-level thread ID
 * @param ptid POSIX thread ID
 */
void PapiManager::registerThread(pid_t tid, pthread_t ptid) {
    pthread_mutex_lock(&mutex_);
    for (auto& t : threads_)
        if (t->getTid() == tid) {
            pthread_mutex_unlock(&mutex_);
            return;
        }

    threads_.push_back(new ThreadInfo(tid, ptid));
    pthread_mutex_unlock(&mutex_);
}

/**
 * @brief Marks a thread as finished, stopping its counter updates.
 * @param ptid POSIX thread ID of the completed thread
 */
void PapiManager::markThreadFinished(pthread_t ptid) {
    pthread_mutex_lock(&mutex_);
    for (auto& t : threads_)
        if (pthread_equal(t->getPthreadId(), ptid)) {
            t->markFinished();
            break;
        }
    pthread_mutex_unlock(&mutex_);
}

/**
 * @brief Updates and logs performance counter deltas for all active threads.
 *
 * Threads are tagged as:
 * - `[MAIN]` for the main thread
 * - `[MONITOR]` for the monitoring thread
 * - `[THREAD]` for all other registered threads
 *
 * @param timestamp Timestamp string used for logging
 */
void PapiManager::updateAllThreads(const char* timestamp) {
    pthread_mutex_lock(&mutex_);
    for (auto& t : threads_)
        if (!t->isFinished()) {
            const char* tag =
                pthread_equal(t->getPthreadId(), monitorThreadId_) ? "[MONITOR]" :
                pthread_equal(t->getPthreadId(), mainThreadId_)     ? "[MAIN]" :
                                                                       "[THREAD]";
            t->printDelta(timestamp, tag);
        }
    pthread_mutex_unlock(&mutex_);
}

/**
 * @brief Logs the final cumulative performance counters for all threads.
 *
 * Typically invoked at the end of execution to report full thread summaries.
 */
void PapiManager::printFinalSummary() {
    char ts[64];
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    snprintf(ts, sizeof(ts), "%lld", (long long)now.tv_sec * 1000 + now.tv_nsec / 1000000);

    pthread_mutex_lock(&mutex_);
    for (auto& t : threads_) {
            const char* tag =
                pthread_equal(t->getPthreadId(), monitorThreadId_) ? "[MONITOR]" :
                pthread_equal(t->getPthreadId(), mainThreadId_)     ? "[MAIN]" :
                                                                       "[THREAD]";
            t->printCumulative(ts, tag);
    }
    pthread_mutex_unlock(&mutex_);
}
