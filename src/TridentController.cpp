#include "TridentController.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <numeric>

// Branch prediction hints
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

// Initialize reference in the initializer list
TridentController::TridentController() 
    : power_(PowerUtils::CpuManager::getInstance()) 
{
    try {
        // 1. Parse Frequencies (Input is KHz integer string, e.g., "2400000")
        uint64_t max_khz = std::stoull(getEnv("URJA_TRIDENT_MAX_FREQ"));
        uint64_t mid_khz = std::stoull(getEnv("URJA_TRIDENT_MID_FREQ"));
        uint64_t min_khz = std::stoull(getEnv("URJA_TRIDENT_MIN_FREQ"));

        // 2. Store directly as double KHz (No division by 1M)
        max_freq_khz_ = static_cast<double>(max_khz);
        mid_freq_khz_ = static_cast<double>(mid_khz);
        min_freq_khz_ = static_cast<double>(min_khz);

        // 3. Parse Thresholds & Hysteresis
        lower_threshold_ = std::stof(getEnv("URJA_TRIDENT_LOWER_THRESHOLD", "0.03"));
        upper_threshold_ = std::stof(getEnv("URJA_TRIDENT_UPPER_THRESHOLD", "0.08"));
        hysteresis_      = std::stof(getEnv("URJA_TRIDENT_HYSTERESIS", "0.005"));

        // 4. Setup Ring Buffer
        int win_input = std::stoi(getEnv("URJA_TRIDENT_WINDOW_SIZE", "10"));
        window_size_ = (win_input > 0) ? static_cast<size_t>(win_input) : 10;
        history_buffer_.resize(window_size_, 0.0);

        // 5. Initialize Hardware
        power_.init();
        power_.setGovernor("userspace");

        // 6. Set Default State
        current_freq_khz_ = mid_freq_khz_;
        power_.setCpuFrequency(current_freq_khz_);

    } catch (const std::exception& e) {
        std::cerr << "[TridentController] Init failed: " << e.what() << std::endl;
        // Fallback safety defaults (in KHz)
        max_freq_khz_ = 2400000.0; mid_freq_khz_ = 1800000.0; min_freq_khz_ = 1200000.0;
        lower_threshold_ = 0.03f; upper_threshold_ = 0.08f;
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

    // Optimized parsing
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
    const char* status = "U"; // Status: Unchanged
    double next_freq_khz = current_freq_khz_;
    
    double ratio = 0.0;
    double ipc = 0.0;
    double miss_rate = 0.0;
    double avg_ratio = 0.0;

    if (LIKELY(sum_tot_ins_ > 0 && sum_tot_cyc_ > 0)) {
        // 1. Calculate Instantaneous Metrics
        ratio = static_cast<double>(sum_l3_tcm_) / static_cast<double>(sum_tot_ins_);
        ipc   = static_cast<double>(sum_tot_ins_) / static_cast<double>(sum_tot_cyc_);
        miss_rate = static_cast<double>(sum_l3_tcm_) / static_cast<double>(sum_tot_cyc_);

        // 2. Update Moving Average (Ring Buffer) - O(1)
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

        // 3. Trident Logic (3-Level with Hysteresis)
        // Memory Bound -> Lower Freq (Save Power)
        if (avg_ratio > (upper_threshold_ + hysteresis_)) {
            next_freq_khz = min_freq_khz_;
        } 
        // Compute Bound -> Boost Freq (Performance)
        else if (avg_ratio < (lower_threshold_ - hysteresis_)) {
            next_freq_khz = max_freq_khz_;
        } 
        // Balanced -> Mid Freq
        else if (avg_ratio >= (lower_threshold_ + hysteresis_) && 
                 avg_ratio <= (upper_threshold_ - hysteresis_)) {
            next_freq_khz = mid_freq_khz_;
        }
        
        // 4. Apply Change if needed
        if (next_freq_khz != current_freq_khz_) {
            power_.setCpuFrequency(next_freq_khz);
            current_freq_khz_ = next_freq_khz;
            status = "C";
        } else {
            if (std::abs(avg_ratio - ratio) < 0.0001) status = "S";
        }
    } else {
        // Fallback: No instructions executed? Go to Min power.
        if (current_freq_khz_ != min_freq_khz_) {
            next_freq_khz = min_freq_khz_;
            power_.setCpuFrequency(next_freq_khz);
            current_freq_khz_ = next_freq_khz;
            status = "C";
        }
    }

    // 5. Logging
    // We convert KHz to GHz here just for display purposes to match previous output format
    printf("[URJA][%s][TRIDENT][AGGREGATED]> TOT_CYC: %lld, TOT_INS: %lld, L3_TCM: %lld, "
           "RATIO: %.6f, AVG_RATIO: %.6f, IPC: %.3f, MISS_RATE: %.6f, STATUS: %s, FREQ: %.2f GHz\n",
           current_timestamp_.c_str(), 
           sum_tot_cyc_, 
           sum_tot_ins_, 
           sum_l3_tcm_,
           ratio, 
           avg_ratio,
           ipc, 
           miss_rate, 
           status, 
           current_freq_khz_ / 1000000.0);

    // 6. Reset Accumulators
    sum_tot_cyc_ = 0;
    sum_tot_ins_ = 0;
    sum_l3_tcm_ = 0;
}
