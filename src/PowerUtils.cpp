#include "PowerUtils.hpp"
#include <fcntl.h>      // open
#include <unistd.h>     // write, close
#include <charconv>
#include <array>
#include <thread>
#include <string>

namespace PowerUtils {

    CpuManager& CpuManager::getInstance() {
        static CpuManager instance;
        return instance;
    }

    void CpuManager::init() {
        int cpuCount = std::thread::hardware_concurrency(); 
        
        m_freqFds.reserve(cpuCount);

        char pathBuffer[64];

        for (int i = 0; i < cpuCount; ++i) {
            sprintf(pathBuffer, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", i);
            int fd = open(pathBuffer, O_WRONLY);
            if (fd >= 0) m_freqFds.push_back(fd);
        }
    }

    void CpuManager::setGovernor(const char* governor) {
    }

    void CpuManager::setCpuFrequency(double freqKHz) {
        uint64_t val = static_cast<uint64_t>(freqKHz);
        std::array<char, 16> buffer;
        auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), val);
        if (ec == std::errc()) {
            size_t len = ptr - buffer.data();            
            for (int fd : m_freqFds) {
                write(fd, buffer.data(), len);
            }
        }
    }
}