#ifndef MONITOR_MANAGER_HPP
#define MONITOR_MANAGER_HPP

class MonitorManager {
public:
    static void start();
    static void stop();

private:
    static void* monitorLoop(void* arg);
};

#endif
