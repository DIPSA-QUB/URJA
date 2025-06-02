#include "FileLogger.hpp"
#include <fstream>

/**
 * @brief Constructs a FileLogger that appends output to the specified file.
 * 
 * @param filename Path to the log file. The file is opened in append mode.
 */
FileLogger::FileLogger(const std::string& filename) : file_(filename, std::ios::app) {}

/**
 * @brief Logs a plain message with timestamp and tag to the output file.
 * 
 * Format: [URJA][timestamp][tag]> message
 *
 * @param timestamp Timestamp string (e.g., "1686200000000")
 * @param tag Short label identifying the source or severity (e.g., "INIT", "ERROR")
 * @param message Message string to write to the log file
 */
void FileLogger::logLine(const char* timestamp, const std::string& tag, const std::string& message) {
    if (file_.is_open())
        file_ << "[URJA][" << timestamp << "][" << tag << "]> " << message << "\n";
}

/**
 * @brief Logs a list of key-value performance metrics with timestamp and tag.
 * 
 * Format: [URJA][timestamp][tag]> key1: val1, key2: val2, ...
 *
 * @param timestamp Timestamp string for the log entry
 * @param tag Identifier label (e.g., "PAPI", "RAPL")
 * @param kvPairs Vector of metric name-value pairs
 */
void FileLogger::logParams(const char* timestamp, const std::string& tag,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (!file_.is_open()) return;

    file_ << "[URJA][" << timestamp << "][" << tag << "]> ";
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        file_ << kvPairs[i].first << ": " << kvPairs[i].second;
        if (i < kvPairs.size() - 1) file_ << ", ";
    }
    file_ << "\n";
}

/**
 * @brief Logs performance counters along with thread ID and pthread ID.
 * 
 * Format: [URJA][timestamp][tag][tid|pthreadId]> key1: val1, ...
 *
 * @param timestamp Timestamp string
 * @param tag Label indicating thread role or source of data
 * @param tid OS thread ID (e.g., result of syscall `gettid`)
 * @param pthreadId POSIX thread identifier (`pthread_t`)
 * @param kvPairs Vector of performance counter pairs
 */
void FileLogger::logParams(const char* timestamp, const std::string& tag,
    pid_t tid, pthread_t pthreadId,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (!file_.is_open()) return;

    file_ << "[URJA][" << timestamp << "][" << tag << "][" << tid << "|" << (unsigned long)pthreadId << "]> ";
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        file_ << kvPairs[i].first << ": " << kvPairs[i].second;
        if (i < kvPairs.size() - 1) file_ << ", ";
    }
    file_ << "\n";
}