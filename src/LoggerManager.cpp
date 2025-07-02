#include "LogTag.hpp"
#include "LoggerManager.hpp"
#include "StdioLogger.hpp"
#include "FileLogger.hpp"
#include "ExperimentalLogger.hpp"
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
    } else if (mode && strcmp(mode, "experimental") == 0) {
        logger_ = std::make_unique<ExperimentalLogger>();
    } else {
        printf("[URJA][ERROR] URJA_LOGGER not set. Using 'stdio'.\n");
        logger_ = std::make_unique<StdioLogger>();
    }
}

void LoggerManager::logLine(const char* timestamp, LogTag tag, const std::string& message) {
    if (logger_) logger_->logLine(timestamp, tag, message);
}

void LoggerManager::logParams(const char* timestamp, LogTag tag,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (logger_) logger_->logParams(timestamp, tag, kvPairs);
}

void LoggerManager::logParams(const char* timestamp, LogTag tag,
    pid_t tid, pthread_t pthreadId,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (logger_) logger_->logParams(timestamp, tag, tid, pthreadId, kvPairs);
}

void LoggerManager::logParams(const char* timestamp, LogTag tag1, LogTag tag2,
    pid_t tid, pthread_t pthreadId,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (logger_) logger_->logParams(timestamp, tag1, tag2, tid, pthreadId, kvPairs);
}