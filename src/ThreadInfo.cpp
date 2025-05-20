#include "ThreadInfo.hpp"
#include <cstring>
#include <cstdio>

static int events[4] = { PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_L2_DCM, PAPI_TLB_DM };

ThreadInfo::ThreadInfo(pid_t tid, pthread_t pthreadId)
    : tid_(tid), pthreadId_(pthreadId), finished_(false) {
    eventSet_ = PAPI_NULL;
    PAPI_create_eventset(&eventSet_);
    PAPI_assign_eventset_component(eventSet_, 0);
    PAPI_attach(eventSet_, tid_);
    for (int event : events)
        PAPI_add_event(eventSet_, event);
    PAPI_start(eventSet_);
    std::memset(prevValues_, 0, sizeof(prevValues_));
    PAPI_read(eventSet_, prevValues_);
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
    long long curr[4];
    if (PAPI_read(eventSet_, curr) == PAPI_OK) {
        for (int i = 0; i < 4; ++i)
            prevValues_[i] = curr[i];
    }
}

void ThreadInfo::printDelta(const char* timestamp, const char* tag) {
    long long curr[4];
    if (PAPI_read(eventSet_, curr) == PAPI_OK) {
        printf("[URJA][PAPI][%s][%d|%lu]%s> ", timestamp, tid_, (unsigned long)pthreadId_, tag);
        for (int i = 0; i < 4; ++i) {
            char name[128];
            PAPI_event_code_to_name(events[i], name);
            printf("%s: %lld", name, curr[i] - prevValues_[i]);
            if (i < 3) printf(", ");
            prevValues_[i] = curr[i];
        }
        printf("\n");
    }
}

void ThreadInfo::printCumulative(const char* timestamp, const char* tag) {
    long long curr[4];
    if (PAPI_read(eventSet_, curr) == PAPI_OK) {
        printf("[CUMULATIVE][URJA][PAPI][%s][%d|%lu]%s> ", timestamp, tid_, (unsigned long)pthreadId_, tag);
        for (int i = 0; i < 4; ++i) {
            char name[128];
            PAPI_event_code_to_name(events[i], name);
            printf("%s: %lld", name, curr[i]);
            if (i < 3) printf(", ");
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
