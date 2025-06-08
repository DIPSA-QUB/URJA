#ifndef IENERGY_BACKEND_HPP
#define IENERGY_BACKEND_HPP

class IEnergyBackend {
public:
    virtual ~IEnergyBackend() = default;
    virtual void initialize() = 0;
    virtual void monitor(const char* timestamp) = 0;
};

#endif