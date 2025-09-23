#include "PowerUtils.hpp"
#include "LoggerManager.hpp"
#include "LogTag.hpp"

#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>

static std::vector<std::ofstream> cpuFiles;

void PowerUtils::initCpuFiles() {
    int cpuCount = std::thread::hardware_concurrency();
    if (cpuCount <= 0) {
        throw std::runtime_error("Failed to detect CPU count");
    }

    cpuFiles.reserve(cpuCount);
    for (int i = 0; i < cpuCount; ++i) {
        std::string path = "/sys/devices/system/cpu/cpu" + std::to_string(i) + "/cpufreq/scaling_setspeed";
        std::ofstream file(path);
        if (!file) {
            throw std::runtime_error("Cannot open " + path);
        }
        cpuFiles.push_back(std::move(file));
    }
}

void PowerUtils::setFrequencyForAllCPUs(const std::string &freqGHz) {
    for (auto &file : cpuFiles) {
        file.seekp(0);
        file << freqGHz << std::flush;
        if (!file) {
            LoggerManager::getInstance().logLine("POWER-UTILS", LogTag::ERROR, "Failed to write frequency!");
        }
    }
}

void PowerUtils::setGovernor(const std::string& governor) {
    /*
    std::string cmd = "sudo /var/shared/power/bin/set-power-options.sh -c all -g " + governor;
    int ret = system(cmd.c_str());
    if (ret != 0) {
        LoggerManager::getInstance().logLine("POWER-UTILS", LogTag::ERROR, "Failed to set governor!");
    }
    */
}

void PowerUtils::setCpuFrequency(const std::string& freqGHz) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;

    auto t1 = high_resolution_clock::now();
    setFrequencyForAllCPUs(freqGHz);  // efficient batch write
    auto t2 = high_resolution_clock::now();

    auto ms_int = duration_cast<milliseconds>(t2 - t1);
    std::chrono::duration<double, std::milli> ms_double = t2 - t1;

    std::cout << "Time elapsed: " << ms_int.count() << "ms\n";
    std::cout << "Time elapsed (double): " << ms_double.count() << "ms\n";
}


/*void PowerUtils::sysfsWriteFreq(int cpu, const std::string &value) {
    std::string path = "/sys/devices/system/cpu/cpu" + std::to_string(cpu) + "/cpufreq/scaling_setspeed";
    std::ofstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open " + path);
    }
    file << value;
}

void PowerUtils::setGovernor(const std::string& governor) {
    std::string cmd = "sudo /var/shared/power/bin/set-power-options.sh -c all -g " + governor;
    int ret = system(cmd.c_str());
    if (ret != 0) {
        LoggerManager::getInstance().logLine("POWER-UTILS", LogTag::ERROR, "Failed to set governor!");
    }
}*/

/*void PowerUtils::setCpuFrequency(const std::string& freqGHz) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    auto t1 = high_resolution_clock::now();
    std::string cmd = "sudo cpupower frequency-set --max " + freqGHz +"GHz > /dev/null 2>&1";
    int ret = system(cmd.c_str());
    if (ret != 0) {
        LoggerManager::getInstance().logLine("POWER-UTILS", LogTag::ERROR, "Failed to set frequency!");
    }
    auto t2 = high_resolution_clock::now();

    auto ms_int = duration_cast<milliseconds>(t2 - t1);

    duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_int.count() << "ms\n";
    std::cout << ms_double.count() << "ms\n";
}*/