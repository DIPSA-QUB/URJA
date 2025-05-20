#ifndef MONITOR_MANAGER_HPP
#define MONITOR_MANAGER_HPP

/**
 * @class MonitorManager
 * @brief Runs the monitoring thread that logs thread counters and energy.
 */
class MonitorManager {
public:
    static void start();
    static void stop();

private:
    static void* monitorLoop(void* arg);
};

#endif
