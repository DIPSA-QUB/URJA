#include "LoggerManager.hpp"
#include "NaiveLogger.hpp"
#include "PowerUtils.hpp"
#include <cstdio>
#include <iostream>

NaiveLogger::NaiveLogger() {
    // Retrieve environment variables
    const char* max_freq_env = std::getenv("URJA_NAIVE_MAX_FREQ");
    const char* min_freq_env = std::getenv("URJA_NAIVE_MIN_FREQ");
    const char* threshold_env = std::getenv("URJA_NAIVE_THRESHOLD");

    // Check if the environment variables are set
    if (!max_freq_env || !min_freq_env || !threshold_env) {
        std::cerr << "ERROR: One or more required environment variables are not set." << std::endl;
        std::cerr << "Please set URJA_NAIVE_MAX_FREQ, URJA_NAIVE_MIN_FREQ, and URJA_NAIVE_THRESHOLD." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Convert and store the values
    MAX_FREQ = max_freq_env;
    MIN_FREQ = min_freq_env;
    try {
        THRESHOLD = std::stof(threshold_env);
    } catch (const std::invalid_argument& e) {
        std::cerr << "ERROR: Invalid THRESHOLD value. Must be a number." << std::endl;
        exit(EXIT_FAILURE);
    } catch (const std::out_of_range& e) {
        std::cerr << "ERROR: THRESHOLD value out of range." << std::endl;
        exit(EXIT_FAILURE);
    }
    PowerUtils::initCpuFiles();
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
    
    for (const auto& [name, value] : kvPairs) {
        if (name == "PAPI_L3_TCM") {
            SUM_L3_TCM += value;
        } else if (name == "PAPI_TOT_INS") {
            SUM_TOT_INS += value;
        } else if (name == "PAPI_TOT_CYC") {
            SUM_TOT_CYC += value;
        }
    }
    current_global_timestamp_ = timestamp;
}

void NaiveLogger::logParams(const char* timestamp, LogTag tag1, LogTag tag2,
    pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) {}

void NaiveLogger::process() {
    std::string STATUS = "U";
    std::string newFreq = MIN_FREQ;
    double ratio = -1;

    if (SUM_TOT_CYC == 0) {
        if (lastAppliedFreq_ != newFreq) {
            PowerUtils::setCpuFrequency(newFreq);
            lastAppliedFreq_ = newFreq;
            STATUS = "C";
        }
    } else if (SUM_L3_TCM >= 0 && SUM_TOT_INS > 0) {
        ratio = static_cast<double>(SUM_L3_TCM) / static_cast<double>(SUM_TOT_INS);
        if (ratio < THRESHOLD) {
            newFreq = MAX_FREQ;
        } else {
            newFreq = MIN_FREQ;
        }
        if (lastAppliedFreq_ != newFreq) {
            PowerUtils::setCpuFrequency(newFreq);
            lastAppliedFreq_ = newFreq;
            STATUS = "C";
        }
    } else {
        if (lastAppliedFreq_ != newFreq) {
            PowerUtils::setCpuFrequency(newFreq);
            lastAppliedFreq_ = newFreq;
            STATUS = "C";
        }
    }
    //printf("[URJA][%s][PAPI][AGGREGATED]> TOT_CYC: %lld, TOT_INS: %lld, L3_TCM: %lld, RATIO: %.5f, STATUS: %s, FREQ: %sGHz\n",
    //                current_global_timestamp_.c_str(), SUM_TOT_CYC, SUM_TOT_INS, SUM_L3_TCM, ratio, STATUS.c_str(), newFreq.c_str());
                    
    SUM_TOT_CYC = 0;
    SUM_TOT_INS = 0;
    SUM_L3_TCM = 0;
}