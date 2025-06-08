#include "LoggerManager.hpp"
#include "StdioLogger.hpp"
#include <cstdio>

void StdioLogger::logLine(const char* timestamp, LogTag tag, const std::string& message) {
    printf("[URJA][%s][%s]> %s\n", timestamp, toString(tag), message.c_str());
}


void StdioLogger::logParams(const char* timestamp, LogTag tag, const std::vector<std::pair<std::string, long long>>& kvPairs) {
    printf("[URJA][%s][%s]> ", timestamp, toString(tag));
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        printf("%s: %lld", kvPairs[i].first.c_str(), kvPairs[i].second);
        if (i < kvPairs.size() - 1) printf(", ");
    }
    printf("\n");
}

void StdioLogger::logParams(const char* timestamp, LogTag tag,
    pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) {
    printf("[URJA][%s][%s][%d|%lu]> ", timestamp, toString(tag), tid, (unsigned long)pthreadId);
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        printf("%s: %lld", kvPairs[i].first.c_str(), kvPairs[i].second);
        if (i < kvPairs.size() - 1) printf(", ");
    }
    printf("\n");
}

void StdioLogger::logParams(const char* timestamp, LogTag tag1, LogTag tag2,
    pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) {
    printf("[URJA][%s][%s][%s][%d|%lu]> ", timestamp, toString(tag1), toString(tag2), tid, (unsigned long)pthreadId);
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        printf("%s: %lld", kvPairs[i].first.c_str(), kvPairs[i].second);
        if (i < kvPairs.size() - 1) printf(", ");
    }
    printf("\n");
}