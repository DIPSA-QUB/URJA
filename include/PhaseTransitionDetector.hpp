#ifndef PHASE_TRANSITION_DETECTOR_HPP
#define PHASE_TRANSITION_DETECTOR_HPP

#include <map>
#include <deque>
#include <vector>
#include <string>
#include <utility>
#include <cmath>

class RealTimeNormalizer {
private:
    int count;
    double mean;
    double m2; // For Welford's online variance

public:
    RealTimeNormalizer();
    double update(double x);
    double get_normalized(double x);
};

class PhaseTransitionDetector {
private:
    std::map<std::string, RealTimeNormalizer> normalizers;
    double threshold;
    int window_size;
    std::map<std::string, std::deque<double>> buffers;

    std::string detect_trend(const std::string& metric);

public:
    PhaseTransitionDetector(double threshold = 3.0, int window_size = 5);
    
    // Data point structure for input
    struct DataPoint {
        double papi_tot_cyc;
        double papi_tot_ins;
        double papi_l2_dcm;
        double timestamp;
        double frequency;
    };
    
    // Z-scores structure
    struct ZScores {
        double papi_tot_cyc;
        double papi_tot_ins;
        double papi_l2_dcm;
        double timestamp;
        double frequency;
    };
    
    // Trends structure
    struct Trends {
        std::string papi_tot_cyc;
        std::string papi_tot_ins;
        std::string papi_l2_dcm;
    };
    
    // Process a data point and return is_transition, z-scores, and trends
    std::tuple<bool, ZScores, Trends> process(const DataPoint& data_point);
};

#endif // PHASE_TRANSITION_DETECTOR_HPP