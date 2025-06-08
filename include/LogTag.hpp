#ifndef LOG_TAG_HPP
#define LOG_TAG_HPP

enum class LogTag {
    DEBUG,
    ERROR,
    INFO,
    INIT,
    THREAD,
    MONITOR,
    MAIN,
    PAPI,
    ENERGY,
    CUMULATIVE
};

inline const char* toString(LogTag tag) {
    switch (tag) {
        case LogTag::DEBUG:   return "DEBUG";
        case LogTag::ERROR:   return "ERROR";
        case LogTag::INFO:    return "INFO";
        case LogTag::INIT:    return "INIT";
        case LogTag::THREAD:  return "THREAD";
        case LogTag::MONITOR: return "MONITOR";
        case LogTag::MAIN:    return "MAIN";
        case LogTag::PAPI:    return "PAPI";
        case LogTag::ENERGY:  return "ENERGY";
        case LogTag::CUMULATIVE:  return "CUMULATIVE";
        default:              return "UNKNOWN";
    }
}

#endif
