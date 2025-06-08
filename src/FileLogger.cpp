#include "FileLogger.hpp"
#include <fstream>

FileLogger::FileLogger(const std::string& filename) : file_(filename, std::ios::app) {}

void FileLogger::logLine(const char* timestamp, const std::string& tag, const std::string& message) {
    if (file_.is_open())
        file_ << "[URJA][" << timestamp << "][" << tag << "]> " << message << "\n";
}

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