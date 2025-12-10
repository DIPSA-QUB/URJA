#pragma once
#include <vector>
#include <string>

namespace PowerUtils {

    class CpuManager {
    public:
        static CpuManager& getInstance();

        // Must be called once. No error checks.
        void init();

        // Raw speed setters
        void setGovernor(const char* governor); 
        void setCpuFrequency(double freqKHz);

    private:
        CpuManager() = default;
        
        std::vector<int> m_freqFds;
    };
}