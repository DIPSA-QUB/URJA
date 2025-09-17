#include "PowerUtils.hpp"
#include "LoggerManager.hpp"
#include "LogTag.hpp"
#include <cstdlib>
#include <string>

void PowerUtils::setGovernor(const std::string& governor) {
    std::string cmd = "env -u LD_PRELOAD URJA_SKIP_INIT=1 sudo /var/shared/power/bin/set-power-options.sh -c all -g " + governor;
    int ret = system(cmd.c_str());
    if (ret != 0) {
        LoggerManager::getInstance().logLine("POWER-UTILS", LogTag::ERROR, "Failed to set governor!");
    }
}

void PowerUtils::setCpuFrequency(const std::string& freqGHz) {
    std::string cmd = "sudo /var/shared/power/bin/set-power-options.sh -c all -u " + freqGHz +"GHz -d " + freqGHz + "GHz > /dev/null 2>&1";
    //LoggerManager::getInstance().logLine("POWER-UTILS", LogTag::ERROR, cmd);
    int ret = system(cmd.c_str());
    if (ret != 0) {
        LoggerManager::getInstance().logLine("POWER-UTILS", LogTag::ERROR, "Failed to set frequency!");
    }
}