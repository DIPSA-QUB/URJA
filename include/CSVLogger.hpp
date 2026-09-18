#ifndef CSV_LOGGER_HPP
#define CSV_LOGGER_HPP

#include "ILogger.hpp"
#include "LoggerManager.hpp"

#include <fstream>
#include <string>
#include <vector>

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
    void EnsureHeader(std::ofstream& out, std::vector<std::string>& columns,
        const std::string& prefix,
        const std::vector<std::pair<std::string, long long>>& kvPairs);
    void WriteRow(std::ofstream& out, const std::vector<std::string>& columns,
        const std::string& row_prefix,
        const std::vector<std::pair<std::string, long long>>& kvPairs);

    std::ofstream papi_file_;
    std::ofstream energy_file_;
    std::vector<std::string> papi_columns_;
    std::vector<std::string> energy_columns_;
};

#endif
