#ifndef FSLLCM_CONTROLLER_HPP
#define FSLLCM_CONTROLLER_HPP

#include "ILogger.hpp"
#include "PowerUtils.hpp"
#include <string>
#include <vector>
#include <cstdint>

// Frequency Scaling via LLC Misses (FS-LLCM), from:
// Hebbar, R. and Milenkovic, A., 2022. PMU-events-driven DVFS techniques for
// improving energy efficiency of modern processors. ACM TOMPECS, 7(1), pp.1-31.
class FSLLCMController : public ILogger {
public:
    FSLLCMController();

    void logLine(const char* timestamp, const LogTag tag, const std::string& message) override;

    void logParams(const char* timestamp, const LogTag tag,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

    void logParams(const char* timestamp, const LogTag tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

    void logParams(const char* timestamp, const LogTag tag1, const LogTag tag2,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override {}

    void process() override;

private:
    // P_0 (highest frequency) ... P_n (lowest frequency), in kHz.
    std::vector<uint64_t> p_states_khz_;
    double max_mpki_;

    long long sum_tot_ins_ = 0;
    long long sum_llc_miss_ = 0;

    std::string current_timestamp_;
    size_t current_state_index_ = 0;

    PowerUtils::CpuManager& power_;
};

#endif
