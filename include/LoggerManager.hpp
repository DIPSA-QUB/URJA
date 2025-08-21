#ifndef LOGGER_MANAGER_HPP
#define LOGGER_MANAGER_HPP

#include "ILogger.hpp"
#include "LogTag.hpp"
#include <memory>
class LoggerManager {
public:
    static LoggerManager& getInstance();
    void initialize();
    
    void logLine(const char* timestamp, LogTag tag, const std::string& message);
    void logParams(const char* timestamp, LogTag tag,
        const std::vector<std::pair<std::string, long long>>& kvPairs);
    void logParams(const char* timestamp, LogTag tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs);
    void logParams(const char* timestamp, LogTag tag1, LogTag tag2,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs);
    void process();

private:
    LoggerManager();
    std::unique_ptr<ILogger> logger_;
};

#endif
