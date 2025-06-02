#ifndef ILOGGER_HPP
#define ILOGGER_HPP

#include <string>
#include <vector>
#include <utility>

/**
 * @class ILogger
 * @brief Abstract interface for logging structured monitoring output.
 *
 * This interface is implemented by concrete logger classes such as:
 * - FileLogger: Logs output to a file
 * - StdioLogger: Logs output to standard output
 *
 * The logger is responsible for formatting and writing performance counters
 * and energy measurements captured by URJA's monitoring components.
 */
class ILogger {
public:
    /**
     * @brief Virtual destructor for proper cleanup of derived loggers.
     */
    virtual ~ILogger() = default;

    /**
     * @brief Logs a single plain message line with a timestamp and tag.
     * @param timestamp Timestamp string (e.g., in milliseconds).
     * @param tag A short label such as "INIT", "DEBUG", or "ERROR".
     * @param message The log message to be written.
     */
    virtual void logLine(const char* timestamp, const std::string& tag, const std::string& message) = 0;

    /**
     * @brief Logs a list of key-value pairs with a timestamp and tag.
     * @param timestamp Timestamp string.
     * @param tag Label for grouping the metrics (e.g., "PAPI", "RAPL").
     * @param kvPairs A vector of (key, value) pairs representing performance counters or energy readings.
     */
    virtual void logParams(const char* timestamp, const std::string& tag, const std::vector<std::pair<std::string, long long>>& kvPairs) = 0;

    /**
     * @brief Logs key-value pairs along with OS and POSIX thread identifiers.
     * @param timestamp Timestamp string.
     * @param tag Label for the log entry (e.g., "PAPI", "MONITOR").
     * @param tid OS thread ID (usually from `syscall(SYS_gettid)`).
     * @param pthreadId POSIX thread ID (`pthread_t`) as returned by `pthread_create`.
     * @param kvPairs A vector of performance counters or statistics associated with this thread.
     */
    virtual void logParams(const char* timestamp, const std::string& tag, pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) = 0;
};

#endif
