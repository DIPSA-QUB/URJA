#include "StdioLogger.hpp"
#include <cstdio>

/**
 * @brief Logs a plain text message to stdout with timestamp and tag.
 *
 * Format: [URJA][timestamp][tag]> message
 *
 * @param timestamp The timestamp string (e.g., milliseconds since epoch).
 * @param tag A label indicating the log context (e.g., DEBUG, INIT).
 * @param message The message to be printed.
 */
void StdioLogger::logLine(const char* timestamp, const std::string& tag, const std::string& message) {
    printf("[URJA][%s][%s]> %s\n", timestamp, tag.c_str(), message.c_str());
}


/**
 * @brief Logs a key-value list of parameters to stdout with timestamp and tag.
 *
 * Format: [URJA][timestamp][tag]> key1: val1, key2: val2, ...
 *
 * @param timestamp The timestamp string.
 * @param tag A context tag for the log.
 * @param kvPairs A vector of key-value pairs to log.
 */
void StdioLogger::logParams(const char* timestamp, const std::string& tag, const std::vector<std::pair<std::string, long long>>& kvPairs) {
    printf("[URJA][%s][%s]> ", timestamp, tag.c_str());
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        printf("%s: %lld", kvPairs[i].first.c_str(), kvPairs[i].second);
        if (i < kvPairs.size() - 1) printf(", ");
    }
    printf("\n");
}

/**
 * @brief Logs parameters with thread context (TID and pthread ID).
 *
 * Format: [URJA][timestamp][tag][tid|pthread_id]> key1: val1, ...
 *
 * @param timestamp The timestamp string.
 * @param tag A tag indicating the context.
 * @param tid The kernel thread ID (gettid).
 * @param pthreadId The POSIX thread ID.
 * @param kvPairs A vector of key-value metrics.
 */
void StdioLogger::logParams(const char* timestamp, const std::string& tag, pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) {
    printf("[URJA][%s][%s][%d|%lu]> ", timestamp, tag.c_str(), tid, (unsigned long)pthreadId);
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        printf("%s: %lld", kvPairs[i].first.c_str(), kvPairs[i].second);
        if (i < kvPairs.size() - 1) printf(", ");
    }
    printf("\n");
}