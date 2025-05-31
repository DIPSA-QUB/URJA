#include "PapiManager.hpp"
#include <algorithm>
#include <cstdio>

static pthread_t monitorThreadId_;
static pthread_t mainThreadId_;

PapiManager::PapiManager() {
    pthread_mutex_init(&mutex_, nullptr);
}

PapiManager& PapiManager::getInstance() {
    static PapiManager instance;
    return instance;
}

void PapiManager::setMonitorThread(pthread_t id) {
    monitorThreadId_ = id;
}

void PapiManager::initialize() {
    mainThreadId_ = pthread_self();
    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT)
        fprintf(stderr, "PAPI initialization failed.\n");
}

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

void PapiManager::markThreadFinished(pthread_t ptid) {
    pthread_mutex_lock(&mutex_);
    for (auto& t : threads_)
        if (pthread_equal(t->getPthreadId(), ptid)) {
            t->markFinished();
            break;
        }
    pthread_mutex_unlock(&mutex_);
}

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
