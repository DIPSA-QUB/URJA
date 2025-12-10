#include "MonitorManager.hpp"
#include "PapiManager.hpp"
#include "EnergyMonitor.hpp"
#include "LoggerManager.hpp"
#include <unistd.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstdint>
#include <cerrno>

static useconds_t intervalMs = 0;
static volatile int keepRunning = 1;
static pthread_t monitorThread;

static void initSleepDuration() {
    const char* env = std::getenv("URJA_INTERVAL_MS");
    if (env) {
        int msec = std::atoi(env);
        if (msec > 0) {
            intervalMs = msec;
            LoggerManager::getInstance().logLine("INIT", LogTag::DEBUG, "Monitoring interval set to " + std::to_string(msec) +" ms.");
            return;
        } else {
            LoggerManager::getInstance().logLine("INIT", LogTag::ERROR, std::string("Invalid URJA_INTERVAL_MS: ") + env);
        }
    }
    intervalMs = 500;
    LoggerManager::getInstance().logLine("INIT", LogTag::INFO, "URJA_INTERVAL_MS not set or invalid. Using default: 500 ms.");
}

void* MonitorManager::monitorLoop(void*) {
    static bool initialized = false;
    if (!initialized) {
        initSleepDuration();
        EnergyMonitor::getInstance().initialize();
        initialized = true;
    }

    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (tfd == -1) {
        LoggerManager::getInstance().logLine(
            "INIT", LogTag::ERROR, "timerfd_create failed, monitorLoop exiting.");
        return nullptr;
    }

    itimerspec its{};
    its.it_value.tv_sec  = intervalMs / 1000;
    its.it_value.tv_nsec = (intervalMs % 1000) * 1000000L;
    its.it_interval      = its.it_value;  // periodic timer

    if (timerfd_settime(tfd, 0, &its, nullptr) == -1) {
        LoggerManager::getInstance().logLine(
            "INIT", LogTag::ERROR, "timerfd_settime failed, monitorLoop exiting.");
        close(tfd);
        return nullptr;
    }

    while (keepRunning) {
        uint64_t expirations = 0;
        ssize_t n = read(tfd, &expirations, sizeof(expirations));

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            LoggerManager::getInstance().logLine(
                "INIT", LogTag::ERROR, "timerfd read failed, monitorLoop exiting.");
            break;
        }

        if (n != sizeof(expirations) || expirations == 0) {
            continue;
        }

        // Timestamp from steady_clock (monotonic)
        auto now = std::chrono::steady_clock::now();
        std::string timestamp = std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ).count()
        );

        EnergyMonitor::getInstance().monitor(timestamp.c_str());
        PapiManager::getInstance().updateAllThreads(timestamp.c_str());
        LoggerManager::getInstance().process();
    }

    close(tfd);
    return nullptr;
}


void MonitorManager::start() {
    pthread_create(&monitorThread, nullptr, monitorLoop, nullptr);
    pthread_detach(monitorThread);
    PapiManager::getInstance().setMonitorThread(monitorThread);
}

void MonitorManager::stop() {
    keepRunning = 0;
    PapiManager::getInstance().finalize();
}