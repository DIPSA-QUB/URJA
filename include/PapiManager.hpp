#ifndef PAPI_MANAGER_HPP
#define PAPI_MANAGER_HPP

#include <vector>
#include <memory>
#include "ThreadInfo.hpp"

class PapiManager {
public:
    static PapiManager& getInstance();
    void initialize();
    void setMonitorThread(pthread_t id);
    void registerThread(pid_t tid, pthread_t ptid);
    void markThreadFinished(pthread_t ptid);
    void updateAllThreads(const char* timestamp);
    void printFinalSummary();

private:
    PapiManager();
    std::vector<std::unique_ptr<ThreadInfo>> threads_;
    pthread_mutex_t mutex_;
};

#endif
