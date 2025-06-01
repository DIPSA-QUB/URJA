// include/LoggerManager.hpp
#ifndef LOGGER_MANAGER_HPP
#define LOGGER_MANAGER_HPP

#include "ILogger.hpp"

class LoggerManager {
public:
    static LoggerManager& getInstance();
    void initialize();
    void logLine(const char* timestamp, const std::string& tag, const std::string& message);
    void logParams(const char* timestamp, const std::string& tag, const std::vector<std::pair<std::string, long long>>& kvPairs);
    void logParams(const char* timestamp, const std::string& tag, pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs);

private:
    LoggerManager();
    ILogger* logger_;
};

#endif
