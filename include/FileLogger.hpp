#ifndef FILE_LOGGER_HPP
#define FILE_LOGGER_HPP

#include "ILogger.hpp"
#include <fstream>

/**
 * @class FileLogger
 * @brief Concrete implementation of ILogger that writes logs to a file.
 *
 * This logger formats its output to be consistent with URJA's logging style and 
 * appends all logs to a file specified at construction. It supports:
 * - Simple tagged log lines
 * - Key-value counter pairs
 * - Thread-specific performance counter dumps
 *
 * The file is opened in append mode and must be writable by the running user.
 */
class FileLogger : public ILogger {
public:
    /**
     * @brief Constructs a FileLogger with the given output filename.
     * @param filename The path to the log file. The file will be opened in append mode.
     */
    explicit FileLogger(const std::string& filename);

    /**
     * @brief Logs a plain message with timestamp and tag.
     * @param timestamp Timestamp string (typically in ms).
     * @param tag Label for the log (e.g., "INIT", "DEBUG").
     * @param message The message to log.
     */
    void logLine(const char* timestamp, const std::string& tag, const std::string& message) override;

    /**
     * @brief Logs a set of key-value performance counters with a tag and timestamp.
     * @param timestamp Timestamp string (typically in ms).
     * @param tag Label for the log (e.g., "PAPI", "RAPL").
     * @param kvPairs Vector of name-value pairs (e.g., event names and their counts).
     */
    void logParams(const char* timestamp, const std::string& tag,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

    /**
     * @brief Logs per-thread performance counters with thread ID and pthread ID.
     * @param timestamp Timestamp string (typically in ms).
     * @param tag Label for the log (e.g., "PAPI", "MONITOR").
     * @param tid OS-level thread ID (PID).
     * @param pthreadId POSIX thread identifier.
     * @param kvPairs Vector of name-value pairs (e.g., event names and their counts).
     */
    void logParams(const char* timestamp, const std::string& tag,
        pid_t tid, pthread_t pthreadId,
        const std::vector<std::pair<std::string, long long>>& kvPairs) override;

private:
    std::ofstream file_; ///< Output file stream for writing logs.
};

#endif