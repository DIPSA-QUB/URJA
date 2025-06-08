#ifndef FILE_LOGGER_HPP
#define FILE_LOGGER_HPP

#include "ILogger.hpp"
#include <fstream>

class FileLogger : public ILogger {
public:
    explicit FileLogger(const std::string& filename);
    void logLine(const char* timestamp, const std::string& tag, const std::string& message) override;
    void logParams(const char* timestamp, const std::string& tag,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;
    void logParams(const char* timestamp, const std::string& tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

private:
    std::ofstream file_;
};

#endif