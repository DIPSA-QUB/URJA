#include "EnergyMonitor.hpp"
#include "RaplSysfsMonitor.hpp"
#include "RaplShellMonitor.hpp"
#include <cstdio>
#include <cstring>

EnergyMonitor::EnergyMonitor() : backend_(nullptr) {}

EnergyMonitor& EnergyMonitor::getInstance() {
    static EnergyMonitor instance;
    return instance;
}

void EnergyMonitor::initialize() {
    const char* backend = std::getenv("URJA_ENERGY_BACKEND");

    if (backend && std::strcmp(backend, "shell") == 0) {
        auto* shell = new RaplShellMonitor();
        shell->initialize();
        backend_ = shell;
        printf("[URJA][INIT] Using RAPL shell backend.\n");
        return;
    } else if (backend && std::strcmp(backend, "sysfs") == 0) {
        auto* sysfs = new RaplSysfsMonitor();
        sysfs->initialize();

        if (sysfs->domainCount() > 0) {
            backend_ = sysfs;
            printf("[URJA][INIT] Using RAPL sysfs backend.\n");
            return;
        } else {
            delete sysfs;
            if (backend && std::strcmp(backend, "sysfs") == 0) {
                fprintf(stderr, "[URJA][ERROR] Sysfs backend forced, but no RAPL domains found.\n");
                return;
            }
        }
    }
    fprintf(stderr, "[URJA][ERROR] No energy backend specified.\n");
}

void EnergyMonitor::monitor(const char* timestamp) {
    if (backend_)
        backend_->monitor(timestamp);
    else
        fprintf(stderr, "[URJA][ERROR] RAPL monitor not initialized.\n");
}
