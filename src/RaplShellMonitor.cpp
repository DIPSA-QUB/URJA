#include "RaplShellMonitor.hpp"
#include "LoggerManager.hpp"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#define CMD_NAME  "sudo /var/shared/power/bin/rapl_read.2.sh -n"
#define CMD_MAX   "sudo /var/shared/power/bin/rapl_read.2.sh -m"
#define CMD_INST  "sudo /var/shared/power/bin/rapl_read.2.sh -i"


void RaplShellMonitor::initialize() {
    std::vector<std::string> labels = runCommandLines(CMD_NAME);
    std::vector<std::string> maxLines = runCommandLines(CMD_MAX);
    std::vector<std::string> instLines = runCommandLines(CMD_INST);

    if (labels.empty() || maxLines.empty() || instLines.empty()) {
        LoggerManager::getInstance().logLine("INIT", LogTag::ERROR, "Failed to initialize RaplShellMonitor: Incomplete data.");
        return;
    }

    size_t count = std::min({labels.size(), maxLines.size(), instLines.size()});

    domains_.clear();
    for (size_t i = 0; i < count; ++i) {
        Domain d;
        d.label = labels[i];
        try {
            d.maxValue = std::stoull(maxLines[i]);
            d.prevValue = std::stoull(instLines[i]);
        } catch (...) {
            d.maxValue = 0;
            d.prevValue = 0;
        }
        domains_.emplace_back(std::move(d));
    }

    LoggerManager::getInstance().logLine("INIT", LogTag::DEBUG,
        "RaplShellMonitor initialized with " + std::to_string(domains_.size()) + " domains.");
}

void RaplShellMonitor::monitor(const char* timestamp) {
    std::vector<std::string> instLines = runCommandLines(CMD_INST);
    std::vector<std::pair<std::string, long long>> deltas;

    size_t count = std::min(domains_.size(), instLines.size());
    for (size_t i = 0; i < count; ++i) {
        uint64_t curr = 0;
        try {
            curr = std::stoull(instLines[i]);
        } catch (...) {
            continue;
        }

        uint64_t delta;
        if (curr >= domains_[i].prevValue) {
            delta = curr - domains_[i].prevValue;
        } else {
            delta = (domains_[i].maxValue > 0)
                    ? (domains_[i].maxValue - domains_[i].prevValue) + curr
                    : 0;
        }

        deltas.emplace_back(domains_[i].label, static_cast<long long>(delta));
        domains_[i].prevValue = curr;
    }

    LoggerManager::getInstance().logParams(timestamp, LogTag::ENERGY, deltas);
}

std::vector<std::string> RaplShellMonitor::runCommandLines(const std::string& cmd) {
    std::vector<std::string> result;
    FILE* fp = popen(cmd.c_str(), "r");
    if (!fp) return result;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp)) {
        std::string line(buffer);
        line.erase(line.find_last_not_of(" \t\n\r") + 1); // trim
        if (!line.empty()) result.push_back(line);
    }

    pclose(fp);
    return result;
}
