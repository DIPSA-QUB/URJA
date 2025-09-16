#include "LoggerManager.hpp"
#include "NaiveLogger.hpp"
#include "PowerUtils.hpp"
#include <cstdio>

NaiveLogger::NaiveLogger() {
    //PowerUtils::setGovernor("userspace");
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
    
    long long l2_dcm = -1;
    long long tot_ins = -1;

    // Find L2_DCM and TOT_INS in the provided key-value pairs
    for (const auto& [name, value] : kvPairs) {
        if (name == "PAPI_L2_DCM") {
            l2_dcm = value;
        } else if (name == "PAPI_TOT_INS") {
            tot_ins = value;
        }
    }

    if (l2_dcm >= 0 && tot_ins >= 0) {
        std::unique_lock<std::mutex> lock(metricsMutex_);
        collectedThreadMetrics_[tid] = {l2_dcm, tot_ins};
    }
    current_global_timestamp_ = timestamp;
}

void NaiveLogger::logParams(const char* timestamp, LogTag tag1, LogTag tag2,
    pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) {}

void NaiveLogger::process() {
    long long total_l2_dcm = 0;
    long long total_tot_ins = 0;

    std::unique_lock<std::mutex> lock(metricsMutex_);

    if (collectedThreadMetrics_.empty()) {
        printf("[URJA][%s][PAPI][AGGREGATED]> No PAPI data collected in this interval. No frequency change.\n",
                current_global_timestamp_.c_str());
        collectedThreadMetrics_.clear(); // Clear to remove stale data
        return;
    }

    for (const auto& entry : collectedThreadMetrics_) {
        total_l2_dcm += entry.second.first;  // L2_DCM
        total_tot_ins += entry.second.second; // TOT_INS
    }

    collectedThreadMetrics_.clear();
    lock.unlock();

    if (total_l2_dcm >= 0 && total_tot_ins > 0) {
        double ratio = static_cast<double>(total_l2_dcm) / static_cast<double>(total_tot_ins);
        std::string newFreq;

        if (ratio < 0.005) {
            newFreq = "2.8";
        } else {
            newFreq = "0.8";
        }
        
        if (lastAppliedFreq_ != newFreq) {
            //PowerUtils::setCpuFrequency(newFreq);
            lastAppliedFreq_ = newFreq;



            printf("[URJA][%s][PAPI][AGGREGATED]> TOT_INS: %lld, L2_DCM: %lld, RATIO: %.5f, STATUS: Changed, FREQ: %sGHz\n",
                    current_global_timestamp_.c_str(), total_tot_ins, total_l2_dcm, ratio, newFreq.c_str());
        } else {
            printf("[URJA][%s][PAPI][AGGREGATED]> TOT_INS: %lld, L2_DCM: %lld, RATIO: %.5f, STATUS: Unchanged, FREQ: %sGHz\n",
                    current_global_timestamp_.c_str(), total_tot_ins, total_l2_dcm, ratio, newFreq.c_str());
        }
    } else { 
        std::string newFreq = "0.8";

        if (lastAppliedFreq_ != newFreq) {
            //PowerUtils::setCpuFrequency(newFreq);
            lastAppliedFreq_ = newFreq;
            printf("[URJA][%s][PAPI][AGGREGATED]> TOT_INS: %lld, L2_DCM: %lld, RATIO: -1, STATUS: Changed, FREQ: %sGHz\n",
                    current_global_timestamp_.c_str(), total_tot_ins, total_l2_dcm, newFreq.c_str());
        } else {
            printf("[URJA][%s][PAPI][AGGREGATED]> TOT_INS: %lld, L2_DCM: %lld, RATIO: -1, STATUS: Unchanged, FREQ: %sGHz\n",
                    current_global_timestamp_.c_str(), total_tot_ins, total_l2_dcm, newFreq.c_str());
        }
    }
}