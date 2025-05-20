#include "MonitorManager.hpp"
#include "PapiManager.hpp"
#include "EnergyMonitor.hpp"
#include <unistd.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static useconds_t monitorSleepUs = 0;

static volatile int keepRunning = 1;
static pthread_t monitorThread;

static void initSleepDuration() {
    const char* env = std::getenv("URJA_INTERVAL_MS");
    if (env) {
        int val = std::atoi(env);
        if (val > 0) {
            monitorSleepUs = val * 1000; // convert ms to us
            printf("[URJA] Monitoring interval set to %d ms via URJA_INTERVAL_MS\n", val);
        } else {
            fprintf(stderr, "[URJA][WARN] Invalid URJA_INTERVAL_MS: %s\n", env);
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
	struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        char timestamp[64];
        snprintf(timestamp, sizeof(timestamp), "%lld", (long long)start.tv_sec * 1000 + start.tv_nsec / 1000000);

        EnergyMonitor::getInstance().monitor(timestamp);
        PapiManager::getInstance().updateAllThreads(timestamp);

	clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                            (end.tv_nsec - start.tv_nsec) / 1e6;

        printf("[URJA][DEBUG] Monitor loop took %.2f ms\n", elapsed_ms);
        usleep(monitorSleepUs);
    }
    return nullptr;
}

void MonitorManager::start() {
    pthread_create(&monitorThread, nullptr, monitorLoop, nullptr);
    pthread_detach(monitorThread);
}

void MonitorManager::stop() {
    keepRunning = 0;
    usleep(monitorSleepUs);
    PapiManager::getInstance().printFinalSummary();
}
