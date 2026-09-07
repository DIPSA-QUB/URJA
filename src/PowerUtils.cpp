#include "PowerUtils.hpp"
#include <fcntl.h>      // open
#include <unistd.h>     // write, close
#include <charconv>
#include <array>
#include <thread>
#include <string>
#include <glob.h>

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

        glob_t g{};
        glob("/sys/devices/system/cpu/intel_uncore_frequency/package_*_die_*",
             GLOB_NOSORT, nullptr, &g);

        for (size_t i = 0; i < g.gl_pathc; ++i) {
            std::string maxPath = std::string(g.gl_pathv[i]) + "/max_freq_khz";

            int maxFd = open(maxPath.c_str(), O_WRONLY);
            if (maxFd >= 0) m_uncoreMaxFds.push_back(maxFd);
        }
        globfree(&g);
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

    void CpuManager::setUncoreFrequency(double freqKHz) {
        uint64_t val = static_cast<uint64_t>(freqKHz);
        std::array<char, 16> buffer;
        auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), val);
        if (ec != std::errc()) return;
        size_t len = ptr - buffer.data();

        for (int fd : m_uncoreMaxFds) write(fd, buffer.data(), len);
    }
}