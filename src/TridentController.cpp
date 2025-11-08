#include "LoggerManager.hpp"
#include "TridentController.hpp"
#include "PowerUtils.hpp"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <deque>
#include <iostream>

TridentController::TridentController() {
    // Environment configuration
    const char* max_freq_env = std::getenv("URJA_TRIDENT_MAX_FREQ");
    const char* mid_freq_env = std::getenv("URJA_TRIDENT_MID_FREQ");
    const char* min_freq_env = std::getenv("URJA_TRIDENT_MIN_FREQ");
    const char* lower_threshold_env = std::getenv("URJA_TRIDENT_LOWER_THRESHOLD");
    const char* upper_threshold_env = std::getenv("URJA_TRIDENT_UPPER_THRESHOLD");

    // Graceful defaults
    MAX_FREQ = max_freq_env ? max_freq_env : "1.2GHz";
    MID_FREQ = mid_freq_env ? mid_freq_env : "1.0GHz";
    MIN_FREQ = min_freq_env ? min_freq_env : "0.8GHz";

    try {
        LOWER_THRESHOLD = lower_threshold_env ? std::stof(lower_threshold_env) : 0.03f;
        UPPER_THRESHOLD = upper_threshold_env ? std::stof(upper_threshold_env) : 0.08f;
    } catch (...) {
        std::cerr << "[URJA][ERROR] Invalid thresholds, using defaults (0.03 / 0.08)" << std::endl;
        LOWER_THRESHOLD = 0.03f;
        UPPER_THRESHOLD = 0.08f;
    }

    PowerUtils::initCpuFiles();
    PowerUtils::setGovernor("userspace");

    lastAppliedFreq_ = MIN_FREQ;
    lastAvgRatio_ = 0.0;
}

// --- Logging functions ---
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

    for (const auto& [name, value] : kvPairs) {
        if (name == "PAPI_L3_TCM") SUM_L3_TCM += value;
        else if (name == "PAPI_TOT_INS") SUM_TOT_INS += value;
        else if (name == "PAPI_TOT_CYC") SUM_TOT_CYC += value;
    }

    current_global_timestamp_ = timestamp;
}

void TridentController::logParams(const char* timestamp, const LogTag tag1, const LogTag tag2,
                                  pid_t tid, pthread_t pthreadId,
                                  const std::vector<std::pair<std::string, long long>>& kvPairs) {}

// --- Core process function ---
void TridentController::process() {
    std::string STATUS = "U";
    std::string newFreq = lastAppliedFreq_;
    double ratio = -1.0;
    double ipc = 0.0;
    double missRate = 0.0;

    if (SUM_TOT_CYC > 0 && SUM_TOT_INS > 0) {
        ratio = static_cast<double>(SUM_L3_TCM) / static_cast<double>(SUM_TOT_INS);
        ipc = static_cast<double>(SUM_TOT_INS) / static_cast<double>(SUM_TOT_CYC);
        missRate = static_cast<double>(SUM_L3_TCM) / static_cast<double>(SUM_TOT_CYC);

        // --- Moving average for smoothing ---
        ratioHistory_.push_back(ratio);
        if (ratioHistory_.size() > WINDOW_SIZE_) ratioHistory_.pop_front();

        double avgRatio = 0.0;
        for (double r : ratioHistory_) avgRatio += r;
        avgRatio /= ratioHistory_.size();

        // --- Dual-threshold 3-level decision logic ---
        if (avgRatio < LOWER_THRESHOLD - HYSTERESIS_) {
            newFreq = MAX_FREQ;
        } else if (avgRatio > UPPER_THRESHOLD + HYSTERESIS_) {
            newFreq = MIN_FREQ;
        } else if (avgRatio >= LOWER_THRESHOLD + HYSTERESIS_ &&
                   avgRatio <= UPPER_THRESHOLD - HYSTERESIS_) {
            newFreq = MID_FREQ;
        }

        // --- Avoid redundant changes ---
        if (fabs(avgRatio - lastAvgRatio_) < 0.0005 && newFreq == lastAppliedFreq_) {
            STATUS = "S";  // Stable
        } else if (newFreq != lastAppliedFreq_) {
            PowerUtils::setCpuFrequency(newFreq);
            STATUS = "C";  // Changed
        } else {
            STATUS = "N";  // No change
        }

        lastAvgRatio_ = avgRatio;
    } else {
        // Fallback if invalid data
        if (lastAppliedFreq_ != MIN_FREQ) {
            PowerUtils::setCpuFrequency(MIN_FREQ);
            STATUS = "C";
            newFreq = MIN_FREQ;
        }
    }

    printf("[URJA][%s][TRIDENT][AGGREGATED]> TOT_CYC: %lld, TOT_INS: %lld, L3_TCM: %lld, "
           "RATIO: %.6f, IPC: %.3f, MISS_RATE: %.6f, STATUS: %s, FREQ: %s\n",
           current_global_timestamp_.c_str(), SUM_TOT_CYC, SUM_TOT_INS, SUM_L3_TCM,
           ratio, ipc, missRate, STATUS.c_str(), newFreq.c_str());

    // Reset for next aggregation
    SUM_TOT_CYC = 0;
    SUM_TOT_INS = 0;
    SUM_L3_TCM = 0;
    lastAppliedFreq_ = newFreq;
}
