#ifndef STDIO_LOGGER_HPP
#define STDIO_LOGGER_HPP

#include "ILogger.hpp"

class StdioLogger : public ILogger {
public:
    void logLine(const char* timestamp, const std::string& tag, const std::string& message) override;
    void logParams(const char* timestamp, const std::string& tag,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;
    void logParams(const char* timestamp, const std::string& tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;   
};

#endif