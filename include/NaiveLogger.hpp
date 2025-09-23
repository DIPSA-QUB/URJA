#ifndef EXPERIMENTAL_LOGGER_HPP
#define EXPERIMENTAL_LOGGER_HPP

#include "ILogger.hpp"
#include "LoggerManager.hpp"
#include <map>
#include <mutex>

class NaiveLogger : public ILogger {
public:
    NaiveLogger();
    
    void logLine(const char* timestamp, const LogTag tag, const std::string& message) override;

    void logParams(const char* timestamp, const LogTag tag,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

    void logParams(const char* timestamp, const LogTag tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;   
    
    void logParams(const char* timestamp, const LogTag tag1, const LogTag tag2,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;   

    void process() override;

private:
    std::string MAX_FREQ;
    std::string MIN_FREQ;
    float THRESHOLD;

    std::string current_global_timestamp_; 

    long long SUM_TOT_CYC = 0;
    long long SUM_TOT_INS = 0;
    long long SUM_L3_TCM = 0;

    std::mutex metricsMutex_;
    std::string lastAppliedFreq_ = "";
};

#endif