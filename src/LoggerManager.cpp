#include "LoggerManager.hpp"
#include "StdioLogger.hpp"
#include "FileLogger.hpp"
#include <cstdlib>
#include <cstring>

/**
 * @brief Constructs the LoggerManager with no logger initially selected.
 */
LoggerManager::LoggerManager() : logger_(nullptr) {}

/**
 * @brief Returns the singleton instance of the LoggerManager.
 *
 * Ensures only one logger is used throughout the URJA runtime.
 * @return Reference to the singleton LoggerManager instance.
 */
LoggerManager& LoggerManager::getInstance() {
    static LoggerManager instance;
    return instance;
}

/**
 * @brief Initializes the logger backend based on environment configuration.
 *
 * Recognized environment variables:
 * - `URJA_LOGGER=stdio` → uses `StdioLogger` (logs to stdout)
 * - `URJA_LOGGER=file` and `URJA_LOG_FILE=/path/to/log` → uses `FileLogger`
 *
 * Logs an error and exits if configuration is invalid or incomplete.
 */
void LoggerManager::initialize() {
    const char* mode = std::getenv("URJA_LOGGER");
    if (mode && strcmp(mode, "file") == 0) {
        const char* file = std::getenv("URJA_LOG_FILE");
        if (file) {
            logger_ = new FileLogger(file);
        } else {
            fprintf(stderr, "[URJA][ERROR] URJA_LOG_FILE not set.\n");
            exit(1);
        }
    } else if (mode && strcmp(mode, "stdio") == 0) {
        logger_ = new StdioLogger();
    } else {
        printf("[URJA][ERROR] URJA_LOGGER not set.\n");
        exit(1);
    }
}

/**
 * @brief Logs a plain message line via the configured logger.
 *
 * @param timestamp Timestamp string for this log entry
 * @param tag Descriptive label (e.g., "DEBUG", "ERROR")
 * @param message The message to be logged
 */
void LoggerManager::logLine(const char* timestamp, const std::string& tag, const std::string& message) {
    if (logger_) logger_->logLine(timestamp, tag, message);
}

/**
 * @brief Logs key-value pairs (metrics) with a tag and timestamp.
 *
 * @param timestamp Timestamp for the log entry
 * @param tag Context or category label (e.g., "PAPI", "RAPL")
 * @param kvPairs Vector of (key, value) metric pairs
 */
void LoggerManager::logParams(const char* timestamp, const std::string& tag,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (logger_) logger_->logParams(timestamp, tag, kvPairs);
}

/**
 * @brief Logs thread-specific key-value metrics with thread identifiers.
 *
 * @param timestamp Timestamp for the log entry
 * @param tag Thread role or type label (e.g., "[MAIN]", "[MONITOR]")
 * @param tid OS thread ID
 * @param pthreadId POSIX thread ID
 * @param kvPairs Vector of (key, value) metric pairs
 */
void LoggerManager::logParams(const char* timestamp, const std::string& tag, 
    pid_t tid, pthread_t pthreadId,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (logger_) logger_->logParams(timestamp, tag, tid, pthreadId, kvPairs);
}