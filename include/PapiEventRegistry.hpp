#ifndef PAPI_EVENT_REGISTRY_HPP
#define PAPI_EVENT_REGISTRY_HPP

#include <vector>
#include <string>

class PapiEventRegistry {
public:
    static PapiEventRegistry& getInstance();
    void initialize();
    const std::vector<int>& getEvents() const;
    const std::vector<std::string>& getEventNames() const;

private:
    PapiEventRegistry();
    std::vector<int> events_;
    std::vector<std::string> eventNames_;
    bool initialized_;
};

#endif
