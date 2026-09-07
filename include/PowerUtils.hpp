#pragma once
#include <vector>
#include <string>

namespace PowerUtils {

    class CpuManager {
    public:
        static CpuManager& getInstance();

        void init();

        void setGovernor(const char* governor);
        void setCpuFrequency(double freqKHz);
        void setUncoreFrequency(double freqKHz);

    private:
        CpuManager() = default;

        std::vector<int> m_freqFds;
        std::vector<int> m_uncoreMaxFds;
    };
}