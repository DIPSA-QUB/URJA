#include "RaplSysfsMonitor.hpp"
#include "LoggerManager.hpp"
#include <glob.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <unordered_set>
#include <iostream>
#include <climits>
RaplSysfsMonitor::~RaplSysfsMonitor() {
    for (const auto& d : domains_) {
        if (d.fd >= 0) {
            close(d.fd);
        }
    }
}

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
        int temp_fd = open(energyPath.c_str(), O_RDONLY);
        if (temp_fd < 0) return;
        std::string basePath = energyPath.substr(0, energyPath.find_last_of('/'));

        char realBuf[PATH_MAX];
        std::string key;
        if (realpath(basePath.c_str(), realBuf)) {
            key = realBuf;
        } else {
            key = basePath;
        }
    
        if (!seen.insert(key).second) {
            close(temp_fd);
            return;
        }
    
        std::string namePath = basePath + "/name";
        std::string maxPath  = basePath + "/max_energy_range_uj";
        std::string label = "unknown";
    
        if (FILE* f = fopen(namePath.c_str(), "r")) {
            char buf[256];
            if (fgets(buf, sizeof(buf), f)) {
                label = buf;
                size_t len = strlen(label.c_str());
                if (len > 0 && label[len-1] == '\n') label.resize(len-1);
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
        
        uint64_t current_energy = readEnergy(temp_fd);

        domains_.push_back({
            formatLabel(label, energyPath),
            temp_fd,
            current_energy,
            max_value
        });
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
    if (domains_.empty()) return;

    std::vector<std::pair<std::string, long long>> energyReadings;
    energyReadings.reserve(domains_.size());

    for (auto& domain : domains_) {
        uint64_t now = readEnergy(domain.fd);
        
        if (now == static_cast<uint64_t>(-1)) {
            continue;
        }

        uint64_t delta;
        
        if (now >= domain.last_value) {
            delta = now - domain.last_value;
        } else if (domain.max_value > 0) {
            delta = (domain.max_value - domain.last_value) + now;
        } else {
            delta = now - domain.last_value; 
        }

        energyReadings.emplace_back(domain.label, static_cast<long long>(delta));
        domain.last_value = now;
    }

    LoggerManager::getInstance().logParams(timestamp, LogTag::ENERGY, energyReadings);
}

uint64_t RaplSysfsMonitor::readEnergy(int fd) {
    char buf[32];
    ssize_t n = pread(fd, buf, sizeof(buf) - 1, 0);
    
    if (n <= 0) return static_cast<uint64_t>(-1);
    
    buf[n] = '\0';
    
    char* end;
    uint64_t val = strtoull(buf, &end, 10);
    
    if (end == buf) return static_cast<uint64_t>(-1);    
    return val;
}

std::string RaplSysfsMonitor::formatLabel(const std::string& label, const std::string& path) {
    size_t idx = path.find("intel-rapl:");
    if (idx == std::string::npos) return label;

    std::string rapl_part = path.substr(idx, path.find('/', idx) - idx);
    std::replace(rapl_part.begin(), rapl_part.end(), ':', '-');

    std::string clean_label = label;
 
    clean_label.erase(std::remove(clean_label.begin(), clean_label.end(), '\n'), clean_label.end());

    size_t dash = clean_label.find_last_of('-');
    if (dash != std::string::npos && dash + 1 < clean_label.size() && isdigit(clean_label[dash + 1])) {
        clean_label = clean_label.substr(0, dash);
    }

    std::string result = rapl_part + "-" + clean_label;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}