#include "CSVLogger.hpp"
#include <cstdio>
#include <cstdlib>

namespace {

std::string csvField(const std::string& value) {
    if (value.find_first_of(",\"\n") == std::string::npos) return value;
    std::string escaped = "\"";
    for (char c : value) {
        if (c == '"') escaped += '"';
        escaped += c;
    }
    escaped += '"';
    return escaped;
}

}

CSVLogger::CSVLogger(const std::string& papiFile, const std::string& energyFile)
    : papi_file_(papiFile, std::ios::out | std::ios::trunc),
      energy_file_(energyFile, std::ios::out | std::ios::trunc) {

    if (!papi_file_.is_open()) {
        fprintf(stderr, "[URJA][ERROR] CSVLogger: failed to open PAPI CSV file '%s'.\n", papiFile.c_str());
        exit(1);
    }
    if (!energy_file_.is_open()) {
        fprintf(stderr, "[URJA][ERROR] CSVLogger: failed to open ENERGY CSV file '%s'.\n", energyFile.c_str());
        exit(1);
    }

    papi_file_ << "timestamp,tag,tid,pthread_id,counter,value\n";
    energy_file_ << "timestamp,tag,domain,value\n";
}

void CSVLogger::logLine(const char* timestamp, const LogTag tag, const std::string& message) {
    fprintf(stderr, "[URJA][%s][%s]> %s\n", timestamp, toString(tag), message.c_str());
}

void CSVLogger::logParams(const char* timestamp, const LogTag tag,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {

    if (tag != LogTag::ENERGY) {
        fprintf(stderr, "[URJA][ERROR] CSVLogger: unexpected tag '%s' for system-wide params, dropping.\n", toString(tag));
        return;
    }
    if (!energy_file_.is_open()) return;

    for (const auto& [name, value] : kvPairs) {
        energy_file_ << timestamp << "," << toString(tag) << "," << csvField(name) << "," << value << "\n";
    }
    energy_file_.flush();
}

void CSVLogger::logParams(const char* timestamp, const LogTag tag,
    pid_t tid, pthread_t pthreadId,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {

    if (!papi_file_.is_open()) return;

    for (const auto& [name, value] : kvPairs) {
        papi_file_ << timestamp << "," << toString(tag) << "," << tid << ","
                    << (unsigned long)pthreadId << "," << csvField(name) << "," << value << "\n";
    }
    papi_file_.flush();
}

void CSVLogger::logParams(const char* timestamp, const LogTag tag1, const LogTag tag2,
    pid_t tid, pthread_t pthreadId,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {

    if (!papi_file_.is_open()) return;

    std::string combinedTag = std::string(toString(tag1)) + "/" + toString(tag2);
    for (const auto& [name, value] : kvPairs) {
        papi_file_ << timestamp << "," << combinedTag << "," << tid << ","
                    << (unsigned long)pthreadId << "," << csvField(name) << "," << value << "\n";
    }
    papi_file_.flush();
}
