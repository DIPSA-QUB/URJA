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

    // Standard Logging (Unused in hot path)
    void logLine(const char* timestamp, const LogTag tag, const std::string& message) override;

    // Hot Path Data Ingestion
    void logParams(const char* timestamp, const LogTag tag,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

    void logParams(const char* timestamp, const LogTag tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

    void logParams(const char* timestamp, const LogTag tag1, const LogTag tag2,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override {}

    // Core Control Logic
    void process() override;

private:
    // Frequencies stored as double KHz (e.g., 2400000.0)
    // We use double because PowerUtils expects double, but the value is KHz magnitude.
    double max_freq_khz_;
    double mid_freq_khz_;
    double min_freq_khz_;

    // Thresholds
    float lower_threshold_;
    float upper_threshold_;
    float hysteresis_;

    // Accumulators
    long long sum_tot_cyc_ = 0;
    long long sum_tot_ins_ = 0;
    long long sum_l3_tcm_ = 0;

    // Circular Buffer for Moving Average (O(1) updates)
    std::vector<double> history_buffer_;
    size_t history_idx_ = 0;
    double history_sum_ = 0.0;
    size_t window_size_ = 5; 
    bool history_filled_ = false;

    // State Tracking
    std::string current_timestamp_;
    double current_freq_khz_ = 0.0;

    // Singleton Reference
    PowerUtils::CpuManager& power_;
};

#endif