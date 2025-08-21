#include "LoggerManager.hpp"
#include "NaiveLogger.hpp"
#include "PowerUtils.hpp"
#include <cstdio>

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
    if(tag != LogTag::MAIN) return;

    long long l2_dcm = -1, tot_ins = -1;

    for (const auto& [name, value] : kvPairs) {
        if (name == "PAPI_L2_DCM") l2_dcm = value;
        else if (name == "PAPI_TOT_INS") tot_ins = value;
    }

    if (l2_dcm >= 0 && tot_ins > 0) {
        double ratio = static_cast<double>(l2_dcm) / static_cast<double>(tot_ins);
        std::string freq = (ratio < 0.005) ? "4.0" : "1.20";
        static std::string lastFreq = ""; // stores last applied frequency

        if (lastFreq != freq) {
            PowerUtils::setCpuFrequency(freq);
            lastFreq = freq;

            printf("[URJA][%s][MAIN][%d|%lu]> TOT_INS: %lld, L2_DCM: %lld, RATIO: %.5f, STATUS: Changed, FREQ: %sGHz\n",
                   timestamp, tid, (unsigned long)pthreadId, tot_ins, l2_dcm, ratio, freq.c_str());
        } else {
            printf("[URJA][%s][MAIN][%d|%lu]> TOT_INS: %lld, L2_DCM: %lld, RATIO: %.5f, STATUS: Unchanged, FREQ: %sGHz\n",
                   timestamp, tid, (unsigned long)pthreadId, tot_ins, l2_dcm, ratio, freq.c_str());
        }
    }
}

void NaiveLogger::logParams(const char* timestamp, LogTag tag1, LogTag tag2,
    pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) {}

void NaiveLogger::process() {
    
}