#include "HwmonMonitor.hpp"
#include "LoggerManager.hpp"
#include <glob.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>

void HwmonMonitor::initialize() {
    glob_t g;
    glob("/sys/class/hwmon/hwmon*/temp*_input", GLOB_NOSORT, nullptr, &g);

    for (size_t i = 0; i < g.gl_pathc; ++i) {
        std::string inputPath = g.gl_pathv[i];

        std::string basePath = inputPath.substr(0, inputPath.find_last_of('/'));
        std::string namePath = basePath + "/name";

        std::ifstream nameFile(namePath);
        std::string sensor = nameFile.is_open() ? std::string((std::istreambuf_iterator<char>(nameFile)), std::istreambuf_iterator<char>()) : "hwmon";
        nameFile.close();
        sensor.erase(std::remove(sensor.begin(), sensor.end(), '\n'), sensor.end());

        std::string labelPath = inputPath;
        std::string label = "temp";
        labelPath.replace(labelPath.find("_input"), 6, "_label");
        std::ifstream labelFile(labelPath);
        if (labelFile.is_open()) {
            label = std::string((std::istreambuf_iterator<char>(labelFile)), std::istreambuf_iterator<char>());
            labelFile.close();
            label.erase(std::remove(label.begin(), label.end(), '\n'), label.end());
        } else {
            size_t start = inputPath.find("temp");
            label = inputPath.substr(start, inputPath.find("_input") - start);
        }

        domains_.push_back({formatLabel(sensor, label, inputPath), inputPath});
    }

    globfree(&g);
    LoggerManager::getInstance().logLine("INIT", LogTag::DEBUG,
        "Loaded " + std::to_string(domains_.size()) + " HWMON temp domains.");
}

void HwmonMonitor::monitor(const char* timestamp) {
    std::vector<std::pair<std::string, long long>> readings;
    for (const auto& domain : domains_) {
        float temp = readTemperature(domain.path);
        readings.emplace_back(domain.label, static_cast<long long>(temp));
    }
    LoggerManager::getInstance().logParams(timestamp, LogTag::ENERGY, readings);
}

float HwmonMonitor::readTemperature(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return -1.0f;

    std::string line;
    std::getline(file, line);
    return std::stof(line) / 1000.0f;
}

std::string HwmonMonitor::formatLabel(const std::string& sensor, const std::string& label, const std::string& path) {
    std::string clean_sensor = sensor;
    std::string clean_label = label;
    std::replace(clean_sensor.begin(), clean_sensor.end(), ' ', '-');
    std::replace(clean_label.begin(), clean_label.end(), ' ', '-');

    return clean_sensor + "-" + clean_label;
}
