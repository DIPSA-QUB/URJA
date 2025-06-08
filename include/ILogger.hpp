#ifndef ILOGGER_HPP
#define ILOGGER_HPP

#include <string>
#include <vector>
#include <utility>
#include "LogTag.hpp"

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void logLine(const char* timestamp, const LogTag tag, const std::string& message) = 0;

    virtual void logParams(const char* timestamp, LogTag tag, 
        const std::vector<std::pair<std::string, long long>>& kvPairs) = 0;

    virtual void logParams(const char* timestamp, LogTag tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) = 0;
    
    virtual void logParams(const char* timestamp, LogTag tag1, LogTag tag2,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) = 0;
};

#endif
