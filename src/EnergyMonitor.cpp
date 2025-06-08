#include "EnergyMonitor.hpp"
#include "RaplSysfsMonitor.hpp"
#include "RaplShellMonitor.hpp"
#include "LoggerManager.hpp"
#include <cstdio>
#include <cstring>
#include <memory>

EnergyMonitor::EnergyMonitor() = default;

EnergyMonitor& EnergyMonitor::getInstance() {
    static EnergyMonitor instance;
    return instance;
}

void EnergyMonitor::initialize() {
    const char* backend = std::getenv("URJA_ENERGY_BACKEND");

    if (backend && std::strcmp(backend, "shell") == 0) {
        std::unique_ptr<RaplShellMonitor> shell = std::make_unique<RaplShellMonitor>();
        shell->initialize();
        backend_ = std::move(shell);
        LoggerManager::getInstance().logLine("INIT", "DEBUG", "Using RAPL shell backend.");
        return;

    } else if (backend && std::strcmp(backend, "sysfs") == 0) {
        std::unique_ptr<RaplSysfsMonitor> sysfs = std::make_unique<RaplSysfsMonitor>();
        sysfs->initialize();

        if (sysfs->domainCount() > 0) {
            backend_ = std::move(sysfs);
            LoggerManager::getInstance().logLine("INIT", "DEBUG", "Using RAPL sysfs backend.");
            return;
        } else {
            LoggerManager::getInstance().logLine("INIT", "ERROR", "Sysfs backend forced, but no RAPL domains found.");
            return;
        }
    }
    LoggerManager::getInstance().logLine("INIT","ERROR", "No energy backend specified.");
}

void EnergyMonitor::monitor(const char* timestamp) {
    if (backend_)
        backend_->monitor(timestamp);
}