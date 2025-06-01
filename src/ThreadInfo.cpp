#include "ThreadInfo.hpp"
#include "LoggerManager.hpp"
#include "PapiEventRegistry.hpp"
#include <cstdlib>      // for std::exit, std::getenv
#include <sstream>      // for std::istringstream
#include <string>       // for std::string
#include <vector>       // for std::vector
#include <iostream>     // for logging fallback (if needed)

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

ThreadInfo::~ThreadInfo() {
    PAPI_stop(eventSet_, nullptr);
    PAPI_cleanup_eventset(eventSet_);
    PAPI_destroy_eventset(&eventSet_);
}

bool ThreadInfo::isFinished() const {
    return finished_;
}

void ThreadInfo::markFinished() {
    finished_ = true;
}

void ThreadInfo::updateCounters() {
    const auto& events = PapiEventRegistry::getInstance().getEvents();
    std::vector<long long> curr(events.size());
    if (PAPI_read(eventSet_, curr.data()) == PAPI_OK) {
        for (size_t i = 0; i < events.size(); ++i)
            prevValues_[i] = curr[i];
    }
}

void ThreadInfo::printDelta(const char* timestamp, const char* tag) {
    const auto& events = PapiEventRegistry::getInstance().getEvents();
    std::vector<long long> curr(events.size());
    if (PAPI_read(eventSet_, curr.data()) == PAPI_OK) {
        //printf("[URJA][PAPI][%s][%d|%lu]%s> ", timestamp, tid_, (unsigned long)pthreadId_, tag);
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

pthread_t ThreadInfo::getPthreadId() const {
    return pthreadId_;
}

pid_t ThreadInfo::getTid() const {
    return tid_;
}
