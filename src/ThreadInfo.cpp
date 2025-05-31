#include "ThreadInfo.hpp"
#include <cstdlib>      // for std::exit, std::getenv
#include <sstream>      // for std::istringstream
#include <string>       // for std::string
#include <vector>       // for std::vector
#include <iostream>     // for logging fallback (if needed)
// Global event list shared by all threads
static std::vector<int> events = { PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_L2_DCM, PAPI_TLB_DM };
static bool events_initialized = false;

static void initializeEvents() {
    if (events_initialized) return;

    const char* env = std::getenv("URJA_PAPI_EVENTS");
    if (!env) {
        fprintf(stderr, "[URJA][ERROR] URJA_PAPI_EVENTS not set. Please export it.\n");
        std::exit(1);
    }

    std::istringstream iss(env);
    std::string token;
    while (std::getline(iss, token, ',')) {
        int code;
        if (PAPI_event_name_to_code(token.c_str(), &code) == PAPI_OK) {
            events.push_back(code);
        } else {
            fprintf(stderr, "[URJA][WARN] Invalid PAPI event name: %s\n", token.c_str());
        }
    }

    if (events.empty()) {
        fprintf(stderr, "[URJA][ERROR] No valid PAPI events found. Aborting.\n");
        std::exit(1);
    }

    events_initialized = true;
}

ThreadInfo::ThreadInfo(pid_t tid, pthread_t pthreadId)
    : tid_(tid), pthreadId_(pthreadId), finished_(false) {

    initializeEvents();
    
    eventSet_ = PAPI_NULL;
    PAPI_create_eventset(&eventSet_);
    PAPI_assign_eventset_component(eventSet_, 0);
    PAPI_attach(eventSet_, tid_);
    for (int event : events)
        PAPI_add_event(eventSet_, event);
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
    std::vector<long long> curr(events.size());
    if (PAPI_read(eventSet_, curr.data()) == PAPI_OK) {
        for (size_t i = 0; i < events.size(); ++i)
            prevValues_[i] = curr[i];
    }
}

void ThreadInfo::printDelta(const char* timestamp, const char* tag) {
    std::vector<long long> curr(events.size());
    if (PAPI_read(eventSet_, curr.data()) == PAPI_OK) {
        printf("[URJA][PAPI][%s][%d|%lu]%s> ", timestamp, tid_, (unsigned long)pthreadId_, tag);
        for (size_t i = 0; i < events.size(); ++i) {
            char name[128];
            PAPI_event_code_to_name(events[i], name);
            printf("%s: %lld", name, curr[i]- prevValues_[i]);
            if (i < events.size() - 1) printf(", ");
            prevValues_[i] = curr[i];
        }
        printf("\n");
    }
}

void ThreadInfo::printCumulative(const char* timestamp, const char* tag) {
    std::vector<long long> curr(events.size());
    if (PAPI_read(eventSet_, curr.data()) == PAPI_OK) {
        printf("[CUMULATIVE][URJA][PAPI][%s][%d|%lu]%s> ", timestamp, tid_, (unsigned long)pthreadId_, tag);
        for (size_t i = 0; i < events.size(); ++i) {
            char name[128];
            PAPI_event_code_to_name(events[i], name);
            printf("%s: %lld", name, curr[i]);
            if (i < events.size() - 1) printf(", ");
        }
        printf("\n");
    }
}

pthread_t ThreadInfo::getPthreadId() const {
    return pthreadId_;
}

pid_t ThreadInfo::getTid() const {
    return tid_;
}
