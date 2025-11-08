#ifndef TRIDENT_CONTROLLER_HPP
#define TRIDENT_CONTROLLER_HPP

#include "ILogger.hpp"
#include "LoggerManager.hpp"
#include <map>
#include <mutex>
#include <deque>
#include <string>

class TridentController : public ILogger {
public:
    TridentController();
    
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
    // Frequency levels
    std::string MAX_FREQ;
    std::string MID_FREQ;
    std::string MIN_FREQ;

    // Thresholds and parameters
    float LOWER_THRESHOLD;
    float UPPER_THRESHOLD;
    const double HYSTERESIS_ = 0.0001;
    const size_t WINDOW_SIZE_ = 3;

    // Aggregated counters
    long long SUM_TOT_CYC = 0;
    long long SUM_TOT_INS = 0;
    long long SUM_L3_TCM = 0;

    // State
    std::string current_global_timestamp_;
    std::string lastAppliedFreq_ = "";
    double lastAvgRatio_ = 0.0;

    // History for smoothing
    std::deque<double> ratioHistory_;

    std::mutex metricsMutex_;
};

#endif
