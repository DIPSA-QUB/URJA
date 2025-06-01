// include/ILogger.hpp
#ifndef ILOGGER_HPP
#define ILOGGER_HPP

#include <string>
#include <vector>
#include <utility>

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void logLine(const char* timestamp, const std::string& tag, const std::string& message) = 0;
    virtual void logParams(const char* timestamp, const std::string& tag, const std::vector<std::pair<std::string, long long>>& kvPairs) = 0;
    virtual void logParams(const char* timestamp, const std::string& tag, pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) = 0;
};

#endif
