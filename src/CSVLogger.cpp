#include "CSVLogger.hpp"
#include <algorithm>
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
}

void CSVLogger::logLine(const char* timestamp, const LogTag tag, const std::string& message) {
    fprintf(stderr, "[URJA][%s][%s]> %s\n", timestamp, toString(tag), message.c_str());
}

void CSVLogger::EnsureHeader(std::ofstream& out, std::vector<std::string>& columns,
    const std::string& prefix,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    if (!columns.empty()) return;
    out << prefix;
    for (const auto& [name, value] : kvPairs) {
        (void)value;
        columns.push_back(name);
        out << "," << csvField(name);
    }
    out << "\n";
}

void CSVLogger::WriteRow(std::ofstream& out, const std::vector<std::string>& columns,
    const std::string& row_prefix,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {
    out << row_prefix;
    for (const auto& col : columns) {
        auto it = std::find_if(kvPairs.begin(), kvPairs.end(),
            [&](const auto& kv) { return kv.first == col; });
        out << ",";
        if (it != kvPairs.end()) out << it->second;
    }
    out << "\n";
    out.flush();
}

void CSVLogger::logParams(const char* timestamp, const LogTag tag,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {

    if (tag != LogTag::ENERGY) {
        fprintf(stderr, "[URJA][ERROR] CSVLogger: unexpected tag '%s' for system-wide params, dropping.\n", toString(tag));
        return;
    }
    if (!energy_file_.is_open()) return;

    EnsureHeader(energy_file_, energy_columns_, "timestamp,tag", kvPairs);
    WriteRow(energy_file_, energy_columns_,
        std::string(timestamp) + "," + toString(tag), kvPairs);
}

void CSVLogger::logParams(const char* timestamp, const LogTag tag,
    pid_t tid, pthread_t pthreadId,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {

    if (!papi_file_.is_open()) return;

    EnsureHeader(papi_file_, papi_columns_, "timestamp,tag,tid,pthread_id", kvPairs);
    WriteRow(papi_file_, papi_columns_,
        std::string(timestamp) + "," + toString(tag) + "," + std::to_string(tid) + "," +
            std::to_string((unsigned long)pthreadId),
        kvPairs);
}

void CSVLogger::logParams(const char* timestamp, const LogTag tag1, const LogTag tag2,
    pid_t tid, pthread_t pthreadId,
    const std::vector<std::pair<std::string, long long>>& kvPairs) {

    if (!papi_file_.is_open()) return;

    EnsureHeader(papi_file_, papi_columns_, "timestamp,tag,tid,pthread_id", kvPairs);
    std::string combinedTag = std::string(toString(tag1)) + "/" + toString(tag2);
    WriteRow(papi_file_, papi_columns_,
        std::string(timestamp) + "," + combinedTag + "," + std::to_string(tid) + "," +
            std::to_string((unsigned long)pthreadId),
        kvPairs);
}
