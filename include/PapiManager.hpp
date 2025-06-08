#ifndef PAPI_MANAGER_HPP
#define PAPI_MANAGER_HPP

#include <unordered_map>
#include <memory>
#include <shared_mutex>
#include "ThreadInfo.hpp"

class PapiManager {
public:
    static PapiManager& getInstance();
    void initialize();
    void setMonitorThread(pthread_t id);
    void registerThread(pid_t tid, pthread_t ptid);
    void markThreadFinished(pid_t ptid);
    void updateAllThreads(const char* timestamp);
    void printThreadSummary(pid_t tid);
    void finalize();

private:
    PapiManager();
    std::unordered_map<pid_t, std::unique_ptr<ThreadInfo>> threads_;
    mutable std::shared_mutex threadsMutex_;
};

#endif
