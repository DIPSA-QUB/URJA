#ifndef IRAPL_BACKEND_HPP
#define IRAPL_BACKEND_HPP

/**
 * @interface IRaplBackend
 * @brief Abstract interface for RAPL monitoring backends.
 */
class IRaplBackend {
public:
    virtual ~IRaplBackend() = default;
    virtual void initialize() = 0;
    virtual void monitor(const char* timestamp) = 0;
};

#endif
