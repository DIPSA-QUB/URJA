#include "LoggerManager.hpp"
#include "StdioLogger.hpp"
#include "FileLogger.hpp"
#include <cstdlib>
#include <cstring>
#include <memory>

LoggerManager::LoggerManager() = default;

LoggerManager& LoggerManager::getInstance() {
    static LoggerManager instance;
    return instance;
}

void LoggerManager::initialize() {
    const char* mode = std::getenv("URJA_LOGGER");
    if (mode && strcmp(mode, "file") == 0) {
        const char* file = std::getenv("URJA_LOG_FILE");
        if (file) {
            logger_ = std::make_unique<FileLogger>(file);
        } else {
            fprintf(stderr, "[URJA][ERROR] URJA_LOG_FILE not set.\n");
            exit(1);
        }
    } else if (mode && strcmp(mode, "stdio") == 0) {
        logger_ = std::make_unique<StdioLogger>();
    } else {
        printf("[URJA][ERROR] URJA_LOGGER not set.\n");
        exit(1);
    }
}

void LoggerManager::logLine(const char* timestamp, const std::string& tag, const std::string& message) {
    if (logger_) logger_->logLine(timestamp, tag, message);
}

void LoggerManager::logParams(const char* timestamp, const std::string& tag,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (logger_) logger_->logParams(timestamp, tag, kvPairs);
}

void LoggerManager::logParams(const char* timestamp, const std::string& tag, 
    pid_t tid, pthread_t pthreadId,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (logger_) logger_->logParams(timestamp, tag, tid, pthreadId, kvPairs);
}