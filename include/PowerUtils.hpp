#pragma once
#include <string>
#include <vector>
#include <fstream>

namespace PowerUtils {
    void setGovernor(const std::string& governor);
    void setCpuFrequency(const std::string& freqGHz);

    void initCpuFiles();                  
    void setFrequencyForAllCPUs(const std::string &freqGHz);
}