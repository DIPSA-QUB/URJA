#include "RaplSysfsMonitor.hpp"
#include "LoggerManager.hpp"
#include <glob.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <algorithm>

void RaplSysfsMonitor::initialize() {
    glob_t g;
    memset(&g, 0, sizeof(g));
    
    glob("/sys/class/powercap/intel-rapl:*/energy_uj",
         GLOB_NOSORT, nullptr, &g);
    
    glob("/sys/class/powercap/intel-rapl:*/intel-rapl:*:*/energy_uj",
         GLOB_NOSORT | GLOB_APPEND, nullptr, &g);
    
    std::unordered_set<std::string> seen;
    
    auto addDomain = [this, &seen](const char* path) {
        std::string energyPath(path);
        uint64_t energy = readEnergy(energyPath);
        if (energy == static_cast<uint64_t>(-1))
            return;
    
        std::string basePath = energyPath.substr(0, energyPath.find_last_of('/'));
    
        char realBuf[PATH_MAX];
        std::string key;
        if (realpath(basePath.c_str(), realBuf)) {
            key = realBuf;
        } else {
            key = basePath;
        }
    
        if (!seen.insert(key).second)
            return;
    
        std::string namePath = basePath + "/name";
        std::string maxPath  = basePath + "/max_energy_range_uj";
    
        std::string label = "unknown";
    
        if (FILE* f = fopen(namePath.c_str(), "r")) {
            char buf[256];
            if (fgets(buf, sizeof(buf), f)) {
                label = buf;
                label.erase(std::remove(label.begin(), label.end(), '\n'),
                            label.end());
            }
            fclose(f);
        }
    
        uint64_t max_value = 0;
        if (FILE* f = fopen(maxPath.c_str(), "r")) {
            char buf[64];
            if (fgets(buf, sizeof(buf), f)) {
                max_value = strtoull(buf, nullptr, 10);
            }
            fclose(f);
        }
    
        domains_.push_back({formatLabel(label, energyPath),
                            energyPath,
                            energy,
                            max_value});
    };
    
    for (size_t i = 0; i < g.gl_pathc; ++i) {
        addDomain(g.gl_pathv[i]);
    }
    
    globfree(&g);
    
    LoggerManager::getInstance().logLine(
        "INIT", LogTag::DEBUG,
        "Loaded " + std::to_string(domains_.size()) + " RAPL domains.");
}

void RaplSysfsMonitor::monitor(const char* timestamp) {
    std::vector<std::pair<std::string, long long>> energyReadings;

    for (size_t i = 0; i < domains_.size(); ++i) {
        uint64_t now = readEnergy(domains_[i].path);
        if (now == static_cast<uint64_t>(-1)) continue;

        uint64_t delta;
        if (now >= domains_[i].last_value) {
            delta = now - domains_[i].last_value;
        } else if (domains_[i].max_value > 0) {
            // Wraparound occurred
            delta = (domains_[i].max_value - domains_[i].last_value) + now;
        } else {
            delta = static_cast<uint64_t>(-1); // fallback if max value not known
        }

        energyReadings.emplace_back(domains_[i].label, static_cast<long long>(delta));
        domains_[i].last_value = now;
    }

    LoggerManager::getInstance().logParams(timestamp, LogTag::ENERGY, energyReadings);
}

uint64_t RaplSysfsMonitor::readEnergy(const std::string& path) {
    char buf[32];
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) return static_cast<uint64_t>(-1);
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return static_cast<uint64_t>(-1);
    buf[n] = '\0';
    return strtoull(buf, nullptr, 10);
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