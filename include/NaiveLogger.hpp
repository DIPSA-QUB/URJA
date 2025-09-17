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
    string MAX_FREQ = "2.10";
    string MIN_FREQ = "1.20";

    std::string current_global_timestamp_; 
    std::map<pid_t, std::tuple<long long, long long, long long>> collectedThreadMetrics_;
    std::mutex metricsMutex_;
    std::string lastAppliedFreq_ = "";
};

#endif