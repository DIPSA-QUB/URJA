#include "PapiManager.hpp"
#include "LoggerManager.hpp"
#include <algorithm>
#include <cstdio>
#include <chrono>
#include <unordered_map>
#include <memory>
#include <shared_mutex>


static pthread_t monitorThreadId_;
static pthread_t mainThreadId_;

PapiManager::PapiManager() {}

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
        LoggerManager::getInstance().logLine("INIT", LogTag::ERROR, "PAPI initialization failed.");
}

void PapiManager::registerThread(pid_t tid, pthread_t ptid) {
    std::unique_lock lock(threadsMutex_);
    if (threads_.find(tid) == threads_.end()) {
        threads_[tid] = std::make_unique<ThreadInfo>(tid, ptid);
    }
}

void PapiManager::markThreadFinished(pid_t tid) {
    std::unique_lock lock(threadsMutex_);
    auto it = threads_.find(tid);
    if (it != threads_.end()) {
        it->second->markFinished();
    } else {
        LoggerManager::getInstance().logLine("THREAD", LogTag::ERROR, "Tried to mark unknown TID as finished.");
        return;
    }
}

void PapiManager::updateAllThreads(const char* timestamp) {
    std::shared_lock lock(threadsMutex_);
    for (const auto& [tid, thread] : threads_) {
        if (!thread->isFinished()) {
            LogTag tag =
                pthread_equal(thread->getPthreadId(), monitorThreadId_) ? LogTag::MONITOR :
                pthread_equal(thread->getPthreadId(), mainThreadId_)     ? LogTag::MAIN :
                                                                            LogTag::THREAD;
            thread->printDelta(timestamp, tag);
        }
    }
}


/*void PapiManager::printThreadSummary(pid_t tid) {
    printf("SUNNY: %d\n", tid);
    std::shared_lock lock(threadsMutex_);
    auto it = threads_.find(tid);
    if (it != threads_.end()) {
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
        std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        std::string ts = std::to_string(millis.count());

        LogTag tag =
            pthread_equal(it->second->getPthreadId(), monitorThreadId_) ? LogTag::MONITOR :
            pthread_equal(it->second->getPthreadId(), mainThreadId_)     ? LogTag::MAIN :
                                                                            LogTag::THREAD;

        it->second->printCumulative(ts.c_str(), tag);
    }
}*/

void PapiManager::finalize() {
    std::shared_lock lock(threadsMutex_);

    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    std::string ts = std::to_string(millis.count());

    for (const auto& [tid, thread] : threads_) {
        LogTag tag =
            pthread_equal(thread->getPthreadId(), monitorThreadId_) ? LogTag::MONITOR :
            pthread_equal(thread->getPthreadId(), mainThreadId_)     ? LogTag::MAIN :
                                                                        LogTag::THREAD;

        thread->printCumulative(ts.c_str(), tag);
    }
}