#include "PapiManager.hpp"
#include "LoggerManager.hpp"
#include <algorithm>
#include <cstdio>
#include <chrono>

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
        LoggerManager::getInstance().logLine("INIT","ERROR", "PAPI initialization failed.");
}

void PapiManager::registerThread(pid_t tid, pthread_t ptid) {
    pthread_mutex_lock(&mutex_);
    for (const std::unique_ptr<ThreadInfo>& t : threads_)
        if (t->getTid() == tid) {
            pthread_mutex_unlock(&mutex_);
            return;
        }

    threads_.emplace_back(std::make_unique<ThreadInfo>(tid, ptid));
    pthread_mutex_unlock(&mutex_);
}

void PapiManager::markThreadFinished(pthread_t ptid) {
    pthread_mutex_lock(&mutex_);
    for (const std::unique_ptr<ThreadInfo>& t : threads_)
        if (pthread_equal(t->getPthreadId(), ptid)) {
            t->markFinished();
            break;
        }
    pthread_mutex_unlock(&mutex_);
}

void PapiManager::updateAllThreads(const char* timestamp) {
    pthread_mutex_lock(&mutex_);
    for (const std::unique_ptr<ThreadInfo>& t : threads_)
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
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    std::string ts = std::to_string(millis.count());

    pthread_mutex_lock(&mutex_);
    for (const std::unique_ptr<ThreadInfo>& t : threads_) {
            const char* tag =
                pthread_equal(t->getPthreadId(), monitorThreadId_) ? "[MONITOR]" :
                pthread_equal(t->getPthreadId(), mainThreadId_)     ? "[MAIN]" :
                                                                       "[THREAD]";
            t->printCumulative(ts.c_str(), tag);
    }
    pthread_mutex_unlock(&mutex_);
}
