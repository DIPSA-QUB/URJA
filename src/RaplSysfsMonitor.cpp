#include "RaplSysfsMonitor.hpp"
#include <glob.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <algorithm>

void RaplSysfsMonitor::initialize() {
    glob_t gtop, gsub;

    // Match energy files for top-level and sub-domains
    glob("/sys/devices/virtual/powercap/intel-rapl/intel-rapl:*/energy_uj", GLOB_NOSORT, nullptr, &gtop);
    glob("/sys/devices/virtual/powercap/intel-rapl/intel-rapl:*/intel-rapl:*:*/energy_uj", GLOB_NOSORT, nullptr, &gsub);

    auto process = [this](glob_t& g) {
        for (size_t i = 0; i < g.gl_pathc; ++i) {
            std::string energyPath(g.gl_pathv[i]);
            long long energy = readEnergy(energyPath);
            if (energy == -1) continue;

            // Attempt to read the domain label
            std::string namePath = energyPath.substr(0, energyPath.find_last_of('/')) + "/name";
            FILE* f = fopen(namePath.c_str(), "r");
            std::string label = "unknown";
            if (f) {
                char buf[256];
                if (fgets(buf, sizeof(buf), f)) {
                    label = buf;
                    label.erase(std::remove(label.begin(), label.end(), '\n'), label.end());
                }
                fclose(f);
            }

            domains_.push_back({formatLabel(label, energyPath), energyPath, energy});
        }
    };

    process(gtop);
    process(gsub);
    globfree(&gtop);
    globfree(&gsub);
}

void RaplSysfsMonitor::monitor(const char* timestamp) {
    printf("[URJA][RAPL][%s]> ", timestamp);
    for (size_t i = 0; i < domains_.size(); ++i) {
        long long now = readEnergy(domains_[i].path);
        if (now == -1) continue;
        long long delta = now - domains_[i].last_value;
        printf("%s: %lld", domains_[i].label.c_str(), delta);
        if (i < domains_.size() - 1) printf(", ");
        domains_[i].last_value = now;
    }
    printf("\n");
}

long long RaplSysfsMonitor::readEnergy(const std::string& path) {
    char buf[32];
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) return -1;
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return -1;
    buf[n] = '\0';
    return atoll(buf);
}

std::string RaplSysfsMonitor::formatLabel(const std::string& label, const std::string& path) {
    size_t idx = path.find("intel-rapl:");
    if (idx == std::string::npos) return label;

    std::string rapl_part = path.substr(idx, path.find('/', idx) - idx);
    std::replace(rapl_part.begin(), rapl_part.end(), ':', '-');

    std::string clean_label = label;
    size_t dash = clean_label.find_last_of('-');
    if (dash != std::string::npos && isdigit(clean_label[dash + 1])) {
        clean_label = clean_label.substr(0, dash);
    }

    std::string result = rapl_part + "-" + clean_label;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}