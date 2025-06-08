#include "StdioLogger.hpp"
#include <cstdio>

void StdioLogger::logLine(const char* timestamp, const std::string& tag, const std::string& message) {
    printf("[URJA][%s][%s]> %s\n", timestamp, tag.c_str(), message.c_str());
}


void StdioLogger::logParams(const char* timestamp, const std::string& tag, const std::vector<std::pair<std::string, long long>>& kvPairs) {
    printf("[URJA][%s][%s]> ", timestamp, tag.c_str());
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        printf("%s: %lld", kvPairs[i].first.c_str(), kvPairs[i].second);
        if (i < kvPairs.size() - 1) printf(", ");
    }
    printf("\n");
}

void StdioLogger::logParams(const char* timestamp, const std::string& tag, pid_t tid, pthread_t pthreadId, const std::vector<std::pair<std::string, long long>>& kvPairs) {
    printf("[URJA][%s][%s][%d|%lu]> ", timestamp, tag.c_str(), tid, (unsigned long)pthreadId);
    for (size_t i = 0; i < kvPairs.size(); ++i) {
        printf("%s: %lld", kvPairs[i].first.c_str(), kvPairs[i].second);
        if (i < kvPairs.size() - 1) printf(", ");
    }
    printf("\n");
}