#ifndef CSV_LOGGER_HPP
#define CSV_LOGGER_HPP

#include "ILogger.hpp"
#include "LoggerManager.hpp"

#include <fstream>
#include <string>

class CSVLogger : public ILogger {
public:
    CSVLogger(const std::string& papiFile, const std::string& energyFile);

    void logLine(const char* timestamp, const LogTag tag, const std::string& message) override;

    void logParams(const char* timestamp, const LogTag tag,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

    void logParams(const char* timestamp, const LogTag tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

    void logParams(const char* timestamp, const LogTag tag1, const LogTag tag2,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

private:
    std::ofstream papi_file_;
    std::ofstream energy_file_;
};

#endif
