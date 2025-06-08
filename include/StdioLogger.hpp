#ifndef STDIO_LOGGER_HPP
#define STDIO_LOGGER_HPP

#include "ILogger.hpp"
#include "LoggerManager.hpp"

class StdioLogger : public ILogger {
public:
    void logLine(const char* timestamp, const LogTag tag, const std::string& message) override;
    void logParams(const char* timestamp, const LogTag tag,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;
    void logParams(const char* timestamp, const LogTag tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;   
    void logParams(const char* timestamp, const LogTag tag1, const LogTag tag2,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;   
};

#endif