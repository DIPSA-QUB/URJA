// include/PapiEventRegistry.hpp
#ifndef PAPI_EVENT_REGISTRY_HPP
#define PAPI_EVENT_REGISTRY_HPP

#include <vector>
#include <string>

class PapiEventRegistry {
public:
    static PapiEventRegistry& getInstance();
    void initialize();
    const std::vector<int>& getEvents() const;

private:
    PapiEventRegistry();
    std::vector<int> events_;
    bool initialized_;
};

#endif
