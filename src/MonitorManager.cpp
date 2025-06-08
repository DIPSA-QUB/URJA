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

static useconds_t intervalMs = 0;
static volatile int keepRunning = 1;
static pthread_t monitorThread;

static void initSleepDuration() {
    const char* env = std::getenv("URJA_INTERVAL_MS");
    if (env) {
        int msec = std::atoi(env);
        if (msec > 0) {
            intervalMs = msec;
            LoggerManager::getInstance().logLine("INIT","DEBUG", "Monitoring interval set to " + std::to_string(msec) +" ms.");
        } else {
            LoggerManager::getInstance().logLine("INIT","ERROR", std::string("Invalid URJA_INTERVAL_MS: ") + env);
        }
    }
}

void* MonitorManager::monitorLoop(void*) {
    static bool initialized = false;
    if (!initialized) {
	initSleepDuration();
        EnergyMonitor::getInstance().initialize();
        initialized = true;
    }	
    
    while (keepRunning) {
	    std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

        std::string timestamp = std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                start.time_since_epoch()
            ).count()
        );

        // Run energy and performance monitoring
        EnergyMonitor::getInstance().monitor(timestamp.c_str());
        PapiManager::getInstance().updateAllThreads(timestamp.c_str());

        /*
        // Measure and log the monitoring loop duration
	    std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> elapsed = 
            std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);

        LoggerManager::getInstance().logLine(timestamp, "DEBUG", "Monitoring loop duration: " + std::to_string(elapsed.count()) + " ms");
        */

        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs)); // Wait before next sample
    }
    return nullptr;
}

void MonitorManager::start() {
    pthread_create(&monitorThread, nullptr, monitorLoop, nullptr);
    pthread_detach(monitorThread);
    PapiManager::getInstance().setMonitorThread(monitorThread);
}

void MonitorManager::stop() {
    keepRunning = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    PapiManager::getInstance().printFinalSummary();
}