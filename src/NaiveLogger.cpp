#include "LoggerManager.hpp"
#include "NaiveLogger.hpp"
#include "PowerUtils.hpp"
#include <cstdio>
#include <iostream>

NaiveLogger::NaiveLogger() {
    PowerUtils::setGovernor("userspace");
}

void NaiveLogger::logLine(const char* timestamp, LogTag tag, const std::string& message) {
    printf("[URJA][%s][%s]> %s\n", timestamp, toString(tag), message.c_str());
}

void NaiveLogger::logParams(const char* timestamp, LogTag tag, const std::vector<std::pair<std::string, long long>>& kvPairs) {
    printf("[URJA][%s][%s]> ", timestamp, toString(tag));
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        printf("%s: %lld", kvPairs[i].first.c_str(), kvPairs[i].second);
        if (i < kvPairs.size() - 1) printf(", ");
    }
    printf("\n");
}

void NaiveLogger::logParams(const char* timestamp, LogTag tag,
    pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if(tag == LogTag::MONITOR) return;
    
    long long L3_TCM = -1;
    long long TOT_INS = -1;
    long long TOT_CYC = -1;

    // Find L3_TCM and TOT_INS in the provided key-value pairs
    for (const auto& [name, value] : kvPairs) {
        if (name == "PAPI_L3_TCM") {
            L3_TCM = value;
        } else if (name == "PAPI_TOT_INS") {
            TOT_INS = value;
        } else if (name == "PAPI_TOT_CYC") {
            TOT_CYC = value;
        }
    }

    if (L3_TCM >= 0 && TOT_INS >= 0) {
        std::unique_lock<std::mutex> lock(metricsMutex_);
        collectedThreadMetrics_[tid] = {TOT_CYC, TOT_INS, L3_TCM};
    }
    current_global_timestamp_ = timestamp;
}

void NaiveLogger::logParams(const char* timestamp, LogTag tag1, LogTag tag2,
    pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) {}

void NaiveLogger::process() {
    long long SUM_TOT_CYC = 0;
    long long SUM_TOT_INS = 0;
    long long SUM_L3_TCM = 0;

    std::unique_lock<std::mutex> lock(metricsMutex_);

    if (collectedThreadMetrics_.empty()) {
        printf("[URJA][%s][PAPI][AGGREGATED]> No PAPI data collected in this interval. No frequency change.\n",
                current_global_timestamp_.c_str());
        collectedThreadMetrics_.clear(); // Clear to remove stale data
        return;
    }

    for (const auto& entry : collectedThreadMetrics_) {
        SUM_TOT_CYC += std::get<0>(entry.second); // TOT_CYC
        SUM_TOT_INS += std::get<1>(entry.second); // TOT_INS
        SUM_L3_TCM += std::get<2>(entry.second);  // L3_TCM
    }

    collectedThreadMetrics_.clear();
    lock.unlock();
    if (SUM_TOT_CYC == 0) {
        std::string newFreq = "0.8";

        if (lastAppliedFreq_ != newFreq) {
            PowerUtils::setCpuFrequency(newFreq);
            lastAppliedFreq_ = newFreq;
            printf("[URJA][%s][PAPI][AGGREGATED]> TOT_CYC: %lld, TOT_INS: %lld, L3_TCM: %lld, RATIO: -1, STATUS: Changed, FREQ: %sGHz\n",
                    current_global_timestamp_.c_str(), SUM_TOT_CYC, SUM_TOT_INS, SUM_L3_TCM, newFreq.c_str());
        } else {
            printf("[URJA][%s][PAPI][AGGREGATED]> TOT_CYC: %lld, TOT_INS: %lld, L3_TCM: %lld, RATIO: -1, STATUS: Unchanged, FREQ: %sGHz\n",
                    current_global_timestamp_.c_str(), SUM_TOT_CYC, SUM_TOT_INS, SUM_L3_TCM, newFreq.c_str());
        }
    } else if (SUM_L3_TCM >= 0 && SUM_TOT_INS > 0) {
        double ratio = static_cast<double>(SUM_L3_TCM) / static_cast<double>(SUM_TOT_INS);
        std::string newFreq;

        if (ratio < 0.00005) {
            newFreq = "2.8";
        } else {
            newFreq = "0.8";
        }
        
        if (lastAppliedFreq_ != newFreq) {
            PowerUtils::setCpuFrequency(newFreq);
            lastAppliedFreq_ = newFreq;

            printf("[URJA][%s][PAPI][AGGREGATED]> TOT_CYC: %lld, TOT_INS: %lld, L3_TCM: %lld, RATIO: %.5f, STATUS: Changed, FREQ: %sGHz\n",
                    current_global_timestamp_.c_str(), SUM_TOT_CYC, SUM_TOT_INS, SUM_L3_TCM, ratio, newFreq.c_str());
        } else {
            printf("[URJA][%s][PAPI][AGGREGATED]> TOT_CYC: %lld, TOT_INS: %lld, L3_TCM: %lld, RATIO: %.5f, STATUS: Unchanged, FREQ: %sGHz\n",
                    current_global_timestamp_.c_str(), SUM_TOT_CYC, SUM_TOT_INS, SUM_L3_TCM, ratio, newFreq.c_str());
        }
    } else { 
        std::string newFreq = "0.8";

        if (lastAppliedFreq_ != newFreq) {
            PowerUtils::setCpuFrequency(newFreq);
            lastAppliedFreq_ = newFreq;
            printf("[URJA][%s][PAPI][AGGREGATED]> TOT_CYC: %lld, TOT_INS: %lld, L3_TCM: %lld, RATIO: -1, STATUS: Changed, FREQ: %sGHz\n",
                    current_global_timestamp_.c_str(), SUM_TOT_CYC, SUM_TOT_INS, SUM_L3_TCM, newFreq.c_str());
        } else {
            printf("[URJA][%s][PAPI][AGGREGATED]> TOT_CYC: %lld, TOT_INS: %lld, L3_TCM: %lld, RATIO: -1, STATUS: Unchanged, FREQ: %sGHz\n",
                    current_global_timestamp_.c_str(), SUM_TOT_CYC, SUM_TOT_INS, SUM_L3_TCM, newFreq.c_str());
        }
    }
}