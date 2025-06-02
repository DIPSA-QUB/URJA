#ifndef LOGGER_MANAGER_HPP
#define LOGGER_MANAGER_HPP

#include "ILogger.hpp"

/**
 * @class LoggerManager
 * @brief Singleton class that manages logging backends for the URJA monitoring system.
 *
 * This class acts as a central gateway for all logging operations. It selects and delegates
 * logging behavior to a concrete implementation of `ILogger`, based on the `URJA_LOGGER` 
 * environment variable:
 * - `URJA_LOGGER=stdio` → uses `StdioLogger`
 * - `URJA_LOGGER=file`  → uses `FileLogger` with path from `URJA_LOG_FILE`
 *
 * It supports logging plain messages, performance counters, and thread-specific statistics.
 */
class LoggerManager {
public:
    /**
     * @brief Returns the singleton instance of LoggerManager.
     * @return Reference to the LoggerManager instance.
     */
    static LoggerManager& getInstance();

    /**
     * @brief Initializes the logger backend using environment configuration.
     *
     * This must be called once before any logging occurs.
     * - `URJA_LOGGER` must be set to either "file" or "stdio".
     * - If "file" is selected, `URJA_LOG_FILE` must also be defined.
     */
    void initialize();

    /**
     * @brief Logs a single-line message with a timestamp and tag.
     * @param timestamp Timestamp string (typically in milliseconds).
     * @param tag Short label indicating log category (e.g., "DEBUG", "INIT").
     * @param message Message content to log.
     */
    void logLine(const char* timestamp, const std::string& tag, const std::string& message);

    /**
     * @brief Logs a vector of key-value pairs with a timestamp and tag.
     * @param timestamp Timestamp string.
     * @param tag Label identifying the data type (e.g., "PAPI", "RAPL").
     * @param kvPairs Vector of (event name, value) pairs.
     */
    void logParams(const char* timestamp, const std::string& tag, const std::vector<std::pair<std::string, long long>>& kvPairs);

    /**
     * @brief Logs key-value pairs with associated thread IDs.
     * @param timestamp Timestamp string.
     * @param tag Label identifying the data type or role (e.g., "[MONITOR]", "[THREAD]").
     * @param tid OS thread ID (from `syscall(SYS_gettid)`).
     * @param pthreadId POSIX thread ID (`pthread_t`).
     * @param kvPairs Vector of (event name, value) pairs related to the thread.
     */
    void logParams(const char* timestamp, const std::string& tag, pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs);

private:
     /**
     * @brief Private constructor to enforce singleton usage.
     */
    LoggerManager();
    ILogger* logger_; ///< Pointer to the active logger backend (StdioLogger or FileLogger).
};

#endif
