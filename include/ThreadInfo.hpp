#ifndef THREAD_INFO_HPP
#define THREAD_INFO_HPP

#include <pthread.h>
#include <papi.h>
#include <unistd.h>
#include <vector>

class ThreadInfo {
public:
    ThreadInfo(pid_t tid, pthread_t pthreadId);
    ~ThreadInfo();
    bool isFinished() const;
    void markFinished();
    void updateCounters();
    void printDelta(const char* timestamp, const char* tag);
    void printCumulative(const char* timestamp, const char* tag);
    pthread_t getPthreadId() const;
    pid_t getTid() const;

private:
    pid_t tid_;
    pthread_t pthreadId_;
    bool finished_;
    int eventSet_;
    std::vector<long long> prevValues_;
};

#endif
