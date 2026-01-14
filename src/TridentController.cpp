#include "TridentController.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <numeric>
#include <cmath>

#define LIKELY(x)      __builtin_expect(!!(x), 1)
#define UNLIKELY(x)    __builtin_expect(!!(x), 0)

static std::string getEnv(const char* name, const char* defaultVal = nullptr) {
    const char* val = std::getenv(name);
    if (!val) {
        if (defaultVal) return std::string(defaultVal);
        throw std::runtime_error(std::string("Missing Env Var: ") + name);
    }
    return std::string(val);
}

TridentController::TridentController() : power_(PowerUtils::CpuManager::getInstance()) {
    try {
        uint64_t max_khz = std::stoull(getEnv("URJA_TRIDENT_MAX_FREQ"));
        uint64_t mid_khz = std::stoull(getEnv("URJA_TRIDENT_MID_FREQ"));
        uint64_t min_khz = std::stoull(getEnv("URJA_TRIDENT_MIN_FREQ"));

        max_freq_khz_ = static_cast<double>(max_khz);
        mid_freq_khz_ = static_cast<double>(mid_khz);
        min_freq_khz_ = static_cast<double>(min_khz);

        lower_threshold_ = std::stof(getEnv("URJA_TRIDENT_LOWER_THRESHOLD", "0.03"));
        upper_threshold_ = std::stof(getEnv("URJA_TRIDENT_UPPER_THRESHOLD", "0.08"));
        hysteresis_      = std::stof(getEnv("URJA_TRIDENT_HYSTERESIS", "0.005"));

        int win_input = std::stoi(getEnv("URJA_TRIDENT_WINDOW_SIZE", "10"));
        window_size_ = (win_input > 0) ? static_cast<size_t>(win_input) : 10;
        history_buffer_.resize(window_size_, 0.0);
        
        transition_count_ = 0;

        power_.init();
        power_.setGovernor("userspace");

        current_freq_khz_ = mid_freq_khz_;
        power_.setCpuFrequency(current_freq_khz_);

    } catch (const std::exception& e) {
        std::cerr << "[TridentController] Init failed: " << e.what() << std::endl;
        max_freq_khz_ = 2400000.0;
        mid_freq_khz_ = 1800000.0;
        min_freq_khz_ = 1200000.0;
        lower_threshold_ = 0.03f;
        upper_threshold_ = 0.08f;
        window_size_ = 10;
        history_buffer_.resize(window_size_, 0.0);
    }
}

void TridentController::logLine(const char* timestamp, const LogTag tag, const std::string& message) {
    printf("[URJA][%s][%s]> %s\n", timestamp, toString(tag), message.c_str());
}

void TridentController::logParams(const char* timestamp, const LogTag tag,
                                  const std::vector<std::pair<std::string, long long>>& kvPairs) {
    printf("[URJA][%s][%s]> ", timestamp, toString(tag));
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        printf("%s: %lld", kvPairs[i].first.c_str(), kvPairs[i].second);
        if (i < kvPairs.size() - 1) printf(", ");
    }
    printf("\n");
}

void TridentController::logParams(const char* timestamp, const LogTag tag,
                                  pid_t tid, pthread_t pthreadId,
                                  const std::vector<std::pair<std::string, long long>>& kvPairs) {
    
    if (tag == LogTag::MONITOR) return;

    for (const auto& kv : kvPairs) {
        if (UNLIKELY(kv.first.empty())) continue;
        const char lastChar = kv.first.back();
        switch (lastChar) {
            case 'M': // PAPI_L3_TCM
                sum_l3_tcm_ += kv.second;
                break;
            case 'S': // PAPI_TOT_INS
                sum_tot_ins_ += kv.second;
                break;
            case 'C': // PAPI_TOT_CYC
                sum_tot_cyc_ += kv.second;
                break;
        }
    }
    current_timestamp_ = timestamp;
}

void TridentController::process() {
    const char* status = "U";
    double next_freq_khz = current_freq_khz_;
    
    double ratio = 0.0;
    double ipc = 0.0;
    double miss_rate = 0.0;
    double avg_ratio = 0.0;
    double effective_ratio = 0.0;

    const double noise_floor = 0.1;

    if (LIKELY(sum_tot_ins_ > 0 && sum_tot_cyc_ > 0)) {
        ratio = static_cast<double>(sum_l3_tcm_) / static_cast<double>(sum_tot_ins_);
        ipc   = static_cast<double>(sum_tot_ins_) / static_cast<double>(sum_tot_cyc_);
        miss_rate = static_cast<double>(sum_l3_tcm_) / static_cast<double>(sum_tot_cyc_);

        if (history_filled_) {
            size_t next_neighbor_idx = (history_idx_ + 1) % window_size_;
            double outgoing_val = history_buffer_[history_idx_];
            double neighbor_val = history_buffer_[next_neighbor_idx];

            if (std::abs(outgoing_val - neighbor_val) > noise_floor) {
                transition_count_--;
            }
        }
        if (history_filled_ || history_idx_ > 0) {
            size_t newest_idx = (history_idx_ == 0) ? (window_size_ - 1) : (history_idx_ - 1);
            double newest_val = history_buffer_[newest_idx];

            if (std::abs(ratio - newest_val) > noise_floor) {
                transition_count_++;
            }
        }
        
        if (transition_count_ < 0) transition_count_ = 0;

        double old_val = history_buffer_[history_idx_];
        history_buffer_[history_idx_] = ratio;
        history_sum_ = history_sum_ - old_val + ratio;
        
        history_idx_++;
        if (history_idx_ >= window_size_) {
            history_idx_ = 0;
            history_filled_ = true;
        }

        double count = history_filled_ ? (double)window_size_ : (double)history_idx_;
        if (count == 0) count = 1.0; 
        avg_ratio = history_sum_ / count;

        int transition_limit = 2;
        if (transition_limit < 0) transition_limit = 0;
        if (transition_count_ <= transition_limit) {
            effective_ratio = ratio;
            status = "F"; 
        } else {
            effective_ratio = avg_ratio;
            status = "S";
        }

        if (effective_ratio > (upper_threshold_ + hysteresis_)) {
            next_freq_khz = min_freq_khz_; // Memory Bound
        } 
        else if (effective_ratio < (lower_threshold_ - hysteresis_)) {
            next_freq_khz = max_freq_khz_; // Compute Bound
        } 
        else if (effective_ratio >= (lower_threshold_ + hysteresis_) && 
                 effective_ratio <= (upper_threshold_ - hysteresis_)) {
            next_freq_khz = mid_freq_khz_; // Balanced
        }
        
        if (next_freq_khz != current_freq_khz_) {
            power_.setCpuFrequency(next_freq_khz);
            current_freq_khz_ = next_freq_khz;
            status = (status[0] == 'F') ? "CF" : "CS";
        }
    } else {
        if (current_freq_khz_ != min_freq_khz_) {
            next_freq_khz = min_freq_khz_;
            power_.setCpuFrequency(next_freq_khz);
            current_freq_khz_ = next_freq_khz;
            status = "C_IDLE";
        }
    }

    printf("[URJA][%s][TRIDENT][AGGREGATED]> TOT_CYC: %lld, TOT_INS: %lld, L3_TCM: %lld, "
           "RATIO: %.6f, AVG: %.6f, EFF: %.6f, TRN: %d, STAT: %s, FREQ: %.2f GHz\n",
           current_timestamp_.c_str(), 
           sum_tot_cyc_, 
           sum_tot_ins_, 
           sum_l3_tcm_,
           ratio, 
           avg_ratio,
           effective_ratio,
           transition_count_,
           status, 
           current_freq_khz_ / 1000000.0);

    sum_tot_cyc_ = 0;
    sum_tot_ins_ = 0;
    sum_l3_tcm_ = 0;
}