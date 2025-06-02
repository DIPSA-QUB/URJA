#include "ThreadInfo.hpp"
#include "LoggerManager.hpp"
#include "PapiEventRegistry.hpp"
#include <cstdlib> 
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

/**
 * @brief Constructor that sets up a PAPI event set for the thread.
 *
 * Initializes the performance counters associated with the given thread.
 *
 * @param tid Kernel thread ID (gettid).
 * @param pthreadId POSIX thread ID.
 */
ThreadInfo::ThreadInfo(pid_t tid, pthread_t pthreadId)
    : tid_(tid), pthreadId_(pthreadId), finished_(false) {

    PapiEventRegistry::getInstance().initialize();
    const auto& events = PapiEventRegistry::getInstance().getEvents();
    
    eventSet_ = PAPI_NULL;
    PAPI_create_eventset(&eventSet_);
    PAPI_assign_eventset_component(eventSet_, 0);
    PAPI_attach(eventSet_, tid_);
    for (int event : events) {
        PAPI_add_event(eventSet_, event);
    }
    PAPI_start(eventSet_);
    prevValues_.assign(events.size(), 0);
    PAPI_read(eventSet_, prevValues_.data());
}

/**
 * @brief Destructor that cleans up the PAPI event set.
 */
ThreadInfo::~ThreadInfo() {
    PAPI_stop(eventSet_, nullptr);
    PAPI_cleanup_eventset(eventSet_);
    PAPI_destroy_eventset(&eventSet_);
}

/**
 * @brief Returns whether the thread has been marked as finished.
 */
bool ThreadInfo::isFinished() const {
    return finished_;
}

/**
 * @brief Marks the thread as finished.
 */
void ThreadInfo::markFinished() {
    finished_ = true;
}

/**
 * @brief Updates internal counter state without logging.
 *
 * Used to refresh `prevValues_` based on latest measurements.
 */
void ThreadInfo::updateCounters() {
    const auto& events = PapiEventRegistry::getInstance().getEvents();
    std::vector<long long> curr(events.size());
    if (PAPI_read(eventSet_, curr.data()) == PAPI_OK) {
        for (size_t i = 0; i < events.size(); ++i)
            prevValues_[i] = curr[i];
    }
}

/**
 * @brief Logs the delta (difference) since last update for all PAPI events.
 *
 * @param timestamp Timestamp string in ms.
 * @param tag Logging tag (e.g., "[THREAD]", "[MONITOR]").
 */
void ThreadInfo::printDelta(const char* timestamp, const char* tag) {
    const auto& events = PapiEventRegistry::getInstance().getEvents();
    std::vector<long long> curr(events.size());
    if (PAPI_read(eventSet_, curr.data()) == PAPI_OK) {
        std::vector<std::pair<std::string, long long>> counters;
        for (size_t i = 0; i < events.size(); ++i) {
            char name[128];
            PAPI_event_code_to_name(events[i], name);
            counters.emplace_back(name, curr[i] - prevValues_[i]);
            prevValues_[i] = curr[i];
        }
        LoggerManager::getInstance().logParams(timestamp, tag, tid_, pthreadId_, counters);
    }
}

/**
 * @brief Logs the cumulative values of all PAPI events.
 *
 * @param timestamp Timestamp string in ms.
 * @param tag Unused here; replaced with "PAPI-CUMULATIVE" in log output.
 */
void ThreadInfo::printCumulative(const char* timestamp, const char* tag) {
    const auto& events = PapiEventRegistry::getInstance().getEvents();
    std::vector<long long> curr(events.size());
    std::vector<std::pair<std::string, long long>> counters;
    if (PAPI_read(eventSet_, curr.data()) == PAPI_OK) {
        //printf("[CUMULATIVE][URJA][PAPI][%s][%d|%lu]%s> ", timestamp, tid_, (unsigned long)pthreadId_, tag);
        for (size_t i = 0; i < events.size(); ++i) {
            char name[128];
            PAPI_event_code_to_name(events[i], name);
            counters.emplace_back(name, curr[i]);
        }
        LoggerManager::getInstance().logParams(timestamp, "PAPI-CUMULATIVE", tid_, pthreadId_, counters);
    }
}

/**
 * @brief Returns the POSIX thread ID.
 */
pthread_t ThreadInfo::getPthreadId() const {
    return pthreadId_;
}

/**
 * @brief Returns the kernel thread ID.
 */
pid_t ThreadInfo::getTid() const {
    return tid_;
}
