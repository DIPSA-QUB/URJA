#include "PowerUtils.hpp"
#include <cstdlib>
#include <string>

void PowerUtils::setGovernor(const std::string& governor) {
    std::string cmd = "env -u LD_PRELOAD sudo /var/shared/power/bin/set-power-options.sh -c 0 -g " + governor + " > /dev/null 2>&1";
    system(cmd.c_str());
}

void PowerUtils::setCpuFrequency(const std::string& freqGHz) {
    std::string cmd = "env -u LD_PRELOAD sudo /var/shared/power/bin/set-power-options.sh -c 0 -f " + freqGHz + "GHz > /dev/null 2>&1";
    system(cmd.c_str());
}