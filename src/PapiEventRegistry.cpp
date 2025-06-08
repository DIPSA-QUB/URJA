#include "PapiEventRegistry.hpp"
#include "LoggerManager.hpp"
#include <papi.h>
#include <sstream>
#include <cstdlib>

PapiEventRegistry::PapiEventRegistry() : initialized_(false) {}

PapiEventRegistry& PapiEventRegistry::getInstance() {
    static PapiEventRegistry instance;
    return instance;
}

void PapiEventRegistry::initialize() {
    // Prevent re-initialization
    if (initialized_) return;

    const char* env = std::getenv("URJA_PAPI_EVENTS");
    if (!env) {
        LoggerManager::getInstance().logLine("INIT", "ERROR", "URJA_PAPI_EVENTS not set.");
        std::exit(1);
    }

    // Initialize the PAPI library
    int ret = PAPI_library_init(PAPI_VER_CURRENT);
    if (ret != PAPI_VER_CURRENT && ret > 0) {
        LoggerManager::getInstance().logLine("INIT", "ERROR", "PAPI library version mismatch.");
        exit(1);
    } else if (ret < 0) {
        LoggerManager::getInstance().logLine("INIT", "ERROR", "PAPI initialization failed.");
        exit(1);
    }

    // Parse comma-separated event names
    std::istringstream iss(env);
    std::string token;
    while (std::getline(iss, token, ',')) {
        int code;
        if (PAPI_event_name_to_code(token.c_str(), &code) == PAPI_OK) {
            events_.push_back(code);
            eventNames_.emplace_back(token);
        } else {
            LoggerManager::getInstance().logLine("INIT", "DEBUG", "Invalid PAPI event name: " + token);
        }
    }

    if (events_.empty()) {
        LoggerManager::getInstance().logLine("INIT", "ERROR", "No valid PAPI events found.");
        std::exit(1);
    }

    LoggerManager::getInstance().logLine("INIT", "DEBUG", "PAPI events loaded.");
    initialized_ = true;
}

const std::vector<int>& PapiEventRegistry::getEvents() const {
    return events_;
}

const std::vector<std::string>& PapiEventRegistry::getEventNames() const {
    return eventNames_;
}
