#ifndef STDIO_LOGGER_HPP
#define STDIO_LOGGER_HPP

#include "ILogger.hpp"

/**
 * @class StdioLogger
 * @brief Concrete implementation of ILogger that writes log output to standard output (stdout).
 *
 * This logger provides formatted logging suitable for human-readable diagnostics and debugging.
 * It is selected when the environment variable `URJA_LOGGER` is set to `"stdio"`.
 *
 * Output format:
 * - Plain messages: [URJA][timestamp][tag]> message
 * - Metrics:        [URJA][timestamp][tag]> key1: val1, key2: val2, ...
 * - Per-thread:     [URJA][timestamp][tag][tid|pthreadId]> key1: val1, ...
 */
class StdioLogger : public ILogger {
public:
    /**
     * @brief Logs a plain message to standard output.
     * @param timestamp Timestamp string (e.g., milliseconds since start).
     * @param tag Log category or level (e.g., "INIT", "DEBUG", "ERROR").
     * @param message The message to print.
     */
    void logLine(const char* timestamp, const std::string& tag, const std::string& message) override;

    /**
     * @brief Logs a list of key-value performance metrics to standard output.
     * @param timestamp Timestamp string.
     * @param tag Log context (e.g., "PAPI", "RAPL").
     * @param kvPairs Vector of metric name and value pairs.
     */
    void logParams(const char* timestamp, const std::string& tag,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;
    
    /**
     * @brief Logs per-thread performance metrics with thread identifiers.
     * @param timestamp Timestamp string.
     * @param tag Label indicating thread type or role.
     * @param tid OS thread ID (from `syscall(SYS_gettid)`).
     * @param pthreadId POSIX thread ID (`pthread_t`).
     * @param kvPairs Vector of metric name and value pairs.
     */    
    void logParams(const char* timestamp, const std::string& tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;   
};

#endif