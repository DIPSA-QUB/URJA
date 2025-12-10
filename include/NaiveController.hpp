#ifndef NAIVE_CONTROLLER_HPP
#define NAIVE_CONTROLLER_HPP

#include "ILogger.hpp"
#include "PowerUtils.hpp"
#include <string>
#include <vector>
#include <cstdint>

class NaiveController : public ILogger {
public:
    NaiveController();
    
    void logLine(const char* timestamp, const LogTag tag, const std::string& message) override;

    // Hot path
    void logParams(const char* timestamp, const LogTag tag,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

    void logParams(const char* timestamp, const LogTag tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;   
    
    void logParams(const char* timestamp, const LogTag tag1, const LogTag tag2,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override {}

    void process() override;

private:
    uint64_t max_freq_int_;
    uint64_t min_freq_int_;
    
    // Hysteresis configuration
    float threshold_;
    float margin_; 

    // Accumulators
    long long sum_tot_cyc_ = 0;
    long long sum_tot_ins_ = 0;
    long long sum_l3_tcm_ = 0;

    std::string current_timestamp_;
    uint64_t current_freq_ = 0;

    PowerUtils::CpuManager& power_;
};

#endif