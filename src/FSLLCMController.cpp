#include "FSLLCMController.hpp"
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>

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

FSLLCMController::FSLLCMController() : power_(PowerUtils::CpuManager::getInstance()) {
    try {
        max_mpki_ = std::stod(getEnv("URJA_FSLLCM_MAX", "100"));

        std::string pstatesStr = getEnv("URJA_FSLLCM_PSTATES");
        std::istringstream iss(pstatesStr);
        std::string token;
        while (std::getline(iss, token, ',')) {
            p_states_khz_.push_back(std::stoull(token));
        }
        if (p_states_khz_.size() < 2) {
            throw std::runtime_error("URJA_FSLLCM_PSTATES needs at least 2 P-states (P_0..P_n)");
        }

        power_.init();
        power_.setGovernor("userspace");

        current_state_index_ = 0;
        power_.setCpuFrequency(p_states_khz_[current_state_index_]);

    } catch (const std::exception& e) {
        std::cerr << "[FSLLCMController] Init failed: " << e.what() << std::endl;
        throw;
    }
}

void FSLLCMController::logLine(const char* timestamp, const LogTag tag, const std::string& message) {
    printf("[URJA][%s][%s]> %s\n", timestamp, toString(tag), message.c_str());
}

void FSLLCMController::logParams(const char* timestamp, const LogTag tag,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    printf("[URJA][%s][%s]> ", timestamp, toString(tag));
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        printf("%s: %lld", kvPairs[i].first.c_str(), kvPairs[i].second);
        if (i < kvPairs.size() - 1) printf(", ");
    }
    printf("\n");
}

void FSLLCMController::logParams(const char* timestamp, const LogTag tag,
    pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) {

    if (tag == LogTag::MONITOR) return;

    for (const auto& kv : kvPairs) {
        if (UNLIKELY(kv.first.empty())) continue;

        const char lastChar = kv.first.back();
        switch (lastChar) {
            case 'M': // PAPI_L3_TCM (LLC misses)
                sum_llc_miss_ += kv.second;
                break;
            case 'S': // PAPI_TOT_INS
                sum_tot_ins_ += kv.second;
                break;
            default:
                break;
        }
    }
    current_timestamp_ = timestamp;
}

void FSLLCMController::process() {
    double llc_mpki;
    if (LIKELY(sum_tot_ins_ > 0)) {
        llc_mpki = (static_cast<double>(sum_llc_miss_) / static_cast<double>(sum_tot_ins_)) * 1000.0;
    } else {
        llc_mpki = 0.0;
    }

    double bounded_mpki = std::min(llc_mpki, max_mpki_);
    size_t n = p_states_khz_.size() - 1;
    double raw_index = (bounded_mpki / max_mpki_) * static_cast<double>(n);

    size_t state_index = static_cast<size_t>(std::lround(raw_index));
    if (state_index > n) state_index = n; // guard against rounding past the last P-state

    if (state_index != current_state_index_) {
        power_.setCpuFrequency(p_states_khz_[state_index]);
        current_state_index_ = state_index;
    }

    printf("[URJA][%s][FSLLCM][AGGREGATED]> TOT_INS: %lld, LLC_MISS: %lld, MPKI: %.4f, "
           "STATE: P_%zu/P_%zu, FREQ: %.3f GHz\n",
           current_timestamp_.c_str(),
           sum_tot_ins_,
           sum_llc_miss_,
           llc_mpki,
           state_index, n,
           p_states_khz_[state_index] / 1000000.0);

    sum_tot_ins_ = 0;
    sum_llc_miss_ = 0;
}
