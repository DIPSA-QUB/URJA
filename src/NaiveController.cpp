#include "NaiveController.hpp"
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <iostream>

// Branch prediction hints for GCC/Clang
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

NaiveController::NaiveController(): power_(PowerUtils::CpuManager::getInstance()) {
    try {
        max_freq_int_ = std::stoull(getEnv("URJA_NAIVE_MAX_FREQ"));
        min_freq_int_ = std::stoull(getEnv("URJA_NAIVE_MIN_FREQ"));
        threshold_    = std::stof(getEnv("URJA_NAIVE_THRESHOLD"));
        
        try {
            margin_ = std::stof(getEnv("URJA_NAIVE_MARGIN", "0.005"));
        } catch(...) { margin_ = 0.05f; }

        power_.init();
        power_.setGovernor("userspace");

        current_freq_ = max_freq_int_;
        power_.setCpuFrequency(max_freq_int_);
    } catch (const std::exception& e) {
        std::cerr << "[NaiveController] Init failed: " << e.what() << std::endl;
        throw;
    }
}

void NaiveController::logLine(const char* timestamp, LogTag tag, const std::string& message) {
    printf("[URJA][%s][%s]> %s\n", timestamp, toString(tag), message.c_str());
}

void NaiveController::logParams(const char* timestamp, LogTag tag, const std::vector<std::pair<std::string, long long>>& kvPairs) {
    printf("[URJA][%s][%s]> ", timestamp, toString(tag));
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        printf("%s: %lld", kvPairs[i].first.c_str(), kvPairs[i].second);
        if (i < kvPairs.size() - 1) printf(", ");
    }
    printf("\n");
}

void NaiveController::logParams(const char* timestamp, LogTag tag,
    pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) {
    
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
            default:
                break;
        }
    }
    current_timestamp_ = timestamp;
}

void NaiveController::process() {
    const char* status = "U"; 
    uint64_t next_freq = current_freq_; 

    if (UNLIKELY(sum_tot_ins_ == 0)) {} 
    else {
        double ratio = static_cast<double>(sum_l3_tcm_) / static_cast<double>(sum_tot_ins_);

        if (ratio < (threshold_ - margin_)) {
            next_freq = max_freq_int_;
        } 
        else if (ratio > (threshold_ + margin_)) {
            next_freq = min_freq_int_;
        }
        
        printf("[URJA][%s][PAPI][AGGREGATED]> TOT_CYC: %lld, TOT_INS: %lld, L3_TCM: %lld, RATIO: %.5f, STATUS: %s, FREQ: %lu\n",
           current_timestamp_.c_str(), 
           sum_tot_cyc_,
           sum_tot_ins_,
           sum_l3_tcm_,
           ratio, 
           status, 
           next_freq);
    }

    if (next_freq != current_freq_) {
        if (next_freq == max_freq_int_) {
            power_.setCpuFrequency(max_freq_int_);
        } else {
            power_.setCpuFrequency(min_freq_int_);
        }
        current_freq_ = next_freq;
        status = "C";
    }

    sum_tot_cyc_ = 0;
    sum_tot_ins_ = 0;
    sum_l3_tcm_ = 0;
}