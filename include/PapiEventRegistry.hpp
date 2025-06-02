#ifndef PAPI_EVENT_REGISTRY_HPP
#define PAPI_EVENT_REGISTRY_HPP

#include <vector>
#include <string>

/**
 * @class PapiEventRegistry
 * @brief Singleton class that manages and provides access to PAPI hardware performance event codes.
 *
 * This class loads and stores a list of performance monitoring events (e.g., `PAPI_TOT_CYC`, `PAPI_L2_DCM`)
 * based on the environment variable `URJA_PAPI_EVENTS`. These events are then used by each thread's
 * `ThreadInfo` instance to initialize and monitor PAPI event sets.
 *
 * Usage:
 * - `initialize()` must be called once before accessing the event list.
 * - Event names are provided as a comma-separated string (e.g., `PAPI_TOT_INS,PAPI_TLB_DM`).
 */
class PapiEventRegistry {
public:
    /**
     * @brief Returns the singleton instance of the registry.
     * @return Reference to the single `PapiEventRegistry` object.
     */
    static PapiEventRegistry& getInstance();

    /**
     * @brief Initializes the registry by parsing `URJA_PAPI_EVENTS` and converting event names to codes.
     *
     * Environment variable required:
     * - `URJA_PAPI_EVENTS`: comma-separated list of PAPI event names (e.g., `PAPI_TOT_INS,PAPI_L2_DCM`).
     *
     * Logs errors for invalid events or missing configuration, and exits on failure.
     */
    void initialize();

    /**
     * @brief Returns a constant reference to the list of parsed PAPI event codes.
     * @return Vector of integer event codes usable with PAPI APIs.
     */
    const std::vector<int>& getEvents() const;

private:
    /**
     * @brief Private constructor for enforcing singleton pattern.
     */
    PapiEventRegistry();
    std::vector<int> events_;   ///< List of successfully parsed PAPI event codes.
    bool initialized_;         ///< Indicates whether initialization has been performed.
};

#endif
