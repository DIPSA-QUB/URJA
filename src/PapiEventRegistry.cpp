#include "PapiEventRegistry.hpp"
#include "LoggerManager.hpp"
#include <papi.h>
#include <sstream>
#include <cstdlib>

/**
 * @brief Constructs the PapiEventRegistry and marks it as uninitialized.
 */
PapiEventRegistry::PapiEventRegistry() : initialized_(false) {}

/**
 * @brief Returns the singleton instance of the PapiEventRegistry.
 *
 * Ensures only one registry is used throughout the URJA runtime.
 * @return Reference to the singleton registry.
 */
PapiEventRegistry& PapiEventRegistry::getInstance() {
    static PapiEventRegistry instance;
    return instance;
}

/**
 * @brief Initializes the PAPI library and loads event codes from environment variable.
 *
 * Environment Variable:
 * - `URJA_PAPI_EVENTS` (comma-separated string of PAPI event names)
 *
 * This function performs:
 * - Initialization of the PAPI library
 * - Conversion of event names to integer codes
 * - Logging of successful or failed initialization steps
 *
 * On failure (e.g., missing variable, invalid events), the program exits with error status.
 */
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

/**
 * @brief Returns the list of loaded PAPI event codes.
 *
 * These codes are used to create thread-local event sets in ThreadInfo.
 * @return Vector of PAPI integer event codes.
 */
const std::vector<int>& PapiEventRegistry::getEvents() const {
    return events_;
}
