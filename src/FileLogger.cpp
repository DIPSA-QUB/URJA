#include "FileLogger.hpp"
#include "LoggerManager.hpp"
#include <fstream>

FileLogger::FileLogger(const std::string& filename) : file_(filename, std::ios::app) {}

void FileLogger::logLine(const char* timestamp, const LogTag tag, const std::string& message) {
    if (file_.is_open())
        file_ << "[URJA][" << timestamp << "][" << toString(tag) << "]> " << message << "\n";
}

void FileLogger::logParams(const char* timestamp, const LogTag tag,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (!file_.is_open()) return;

    file_ << "[URJA][" << timestamp << "][" << toString(tag) << "]> ";
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        file_ << kvPairs[i].first << ": " << kvPairs[i].second;
        if (i < kvPairs.size() - 1) file_ << ", ";
    }
    file_ << "\n";
}

void FileLogger::logParams(const char* timestamp, const LogTag tag,
    pid_t tid, pthread_t pthreadId,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (!file_.is_open()) return;

    file_ << "[URJA][" << timestamp << "][" << toString(tag) << "][" << tid << "|" << (unsigned long)pthreadId << "]> ";
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        file_ << kvPairs[i].first << ": " << kvPairs[i].second;
        if (i < kvPairs.size() - 1) file_ << ", ";
    }
    file_ << "\n";
}

void FileLogger::logParams(const char* timestamp, const LogTag tag1, const LogTag tag2,
    pid_t tid, pthread_t pthreadId,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (!file_.is_open()) return;

    file_ << "[URJA][" << timestamp << "][" << toString(tag1) << "][" << toString(tag2) 
          << "][" << tid << "|" << (unsigned long)pthreadId << "]> ";
    
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        file_ << kvPairs[i].first << ": " << kvPairs[i].second;
        if (i < kvPairs.size() - 1) file_ << ", ";
    }
    file_ << "\n";
}