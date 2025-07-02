#include "RaplShellMonitor.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <mutex>
#include <chrono>

#define SHELL_RAPL_CMD "env -u LD_PRELOAD sudo /var/shared/power/bin/rapl_read.sh"

static bool initialized = false;

void RaplShellMonitor::initialize() {
    if (!initialized) {
        prev_ = parseShellOutput();
        initialized = true;
    }
}

std::vector<RaplShellMonitor::ShellDomain> RaplShellMonitor::parseShellOutput() {
    // valgrind <- use for debugging
    std::vector<ShellDomain> result;
    FILE* fp = popen(SHELL_RAPL_CMD, "r");
    if (!fp) {
        perror("popen");
        return result;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // Skip lines not starting with "intel-rapl"
        if (strncmp(line, "intel-rapl", 10) != 0) continue;
        char name[64], index[16], domain[64];
        unsigned long long energy, max_energy;
        if (sscanf(line, " %[^; \t\n] ; %[^; \t\n] ; %[^; \t\n] ; %llu ; %llu",
                   name, index, domain, &energy, &max_energy) == 5) {
            result.push_back({index, domain, energy, max_energy});
        }
    }

    int status = pclose(fp); // Always close the process
    if (status == -1) {
        perror("pclose");
    } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "[URJA][ERROR] Script exited with status %d\n", WEXITSTATUS(status));
    }
    return result;
}

void RaplShellMonitor::monitor(const char* timestamp) {
    auto curr = parseShellOutput();
    printf("[URJA][RAPL][%s]> ", timestamp);

    for (size_t i = 0; i < curr.size(); ++i) {
        unsigned long long delta = 0;
        if (i < prev_.size()) {
            if (curr[i].energy_uj >= prev_[i].energy_uj) {
                delta = curr[i].energy_uj - prev_[i].energy_uj;
            } else {
                // Energy counter wraparound
                delta = (curr[i].max_energy_uj - prev_[i].energy_uj) + curr[i].energy_uj;
            }
        }

        std::string label = curr[i].domain + "_" + curr[i].index;
        std::replace(label.begin(), label.end(), ':', '-');
        std::transform(label.begin(), label.end(), label.begin(), ::toupper);

        printf("%s: %llu", label.c_str(), delta);
        if (i < curr.size() - 1) printf(", ");
    }

    printf("\n");
    prev_ = std::move(curr); // Update for next cycle
}
