#ifndef TRIDENT_CONTROLLER_HPP
#define TRIDENT_CONTROLLER_HPP

#include "ILogger.hpp"
#include "PowerUtils.hpp"
#include <string>
#include <vector>
#include <cmath>

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
        const std::vector<std::pair<std::string, long long>>& kvPairs) override {}

    void process() override;

private:
    double max_freq_khz_;
    double mid_freq_khz_;
    double min_freq_khz_;

    float lower_threshold_;
    float upper_threshold_;
    float hysteresis_;

    long long sum_tot_cyc_ = 0;
    long long sum_tot_ins_ = 0;
    long long sum_l3_tcm_ = 0;

    std::vector<double> history_buffer_;
    size_t history_idx_ = 0;
    double history_sum_ = 0.0;
    size_t window_size_ = 10;
    bool history_filled_ = false;

    int transition_count_ = 0; 
    
    std::string current_timestamp_;
    double current_freq_khz_ = 0.0;

    PowerUtils::CpuManager& power_;
};

#endif