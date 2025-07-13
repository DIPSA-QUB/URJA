#include "PhaseTransitionDetector.hpp"
#include <algorithm>
#include <stdexcept>

RealTimeNormalizer::RealTimeNormalizer() : count(0), mean(0.0), m2(0.0) {}

double RealTimeNormalizer::update(double x) {
    count++;
    double delta = x - mean;
    mean += delta / count;
    double delta2 = x - mean;
    m2 += delta * delta2;
    return get_normalized(x);
}

double RealTimeNormalizer::get_normalized(double x) {
    if (count < 2) {
        return 0.0; // Not enough data to normalize
    }
    double variance = m2 / (count - 1);
    double std = variance > 0.0 ? std::sqrt(variance) : 1.0;
    return (x - mean) / std;
}

PhaseTransitionDetector::PhaseTransitionDetector(double threshold, int window_size)
    : threshold(threshold), window_size(window_size) {
    normalizers["PAPI_TOT_CYC"] = RealTimeNormalizer();
    normalizers["PAPI_TOT_INS"] = RealTimeNormalizer();
    normalizers["PAPI_L2_DCM"] = RealTimeNormalizer();
    buffers["PAPI_TOT_CYC"] = std::deque<double>(window_size);
    buffers["PAPI_TOT_INS"] = std::deque<double>(window_size);
    buffers["PAPI_L2_DCM"] = std::deque<double>(window_size);
}

std::string PhaseTransitionDetector::detect_trend(const std::string& metric) {
    const auto& buffer = buffers[metric];
    if (buffer.size() < 2) {
        return "Insufficient data";
    }
    
    // Compute differences between consecutive values
    std::vector<double> diffs;
    for (size_t i = 1; i < buffer.size(); ++i) {
        diffs.push_back(buffer[i] - buffer[i - 1]);
    }
    
    // Check if all differences are positive or negative
    bool all_positive = std::all_of(diffs.begin(), diffs.end(), [](double d) { return d > 0; });
    bool all_negative = std::all_of(diffs.begin(), diffs.end(), [](double d) { return d < 0; });
    
    if (all_positive) {
        return "Increasing";
    } else if (all_negative) {
        return "Decreasing";
    } else {
        return "Stable";
    }
}

std::tuple<bool, PhaseTransitionDetector::ZScores, PhaseTransitionDetector::Trends>
PhaseTransitionDetector::process(const PhaseTransitionDetector::DataPoint& data_point) {
    ZScores z_scores;
    Trends trends;
    
    // Map input data to metrics
    std::map<std::string, double> data = {
        {"PAPI_TOT_CYC", data_point.papi_tot_cyc},
        {"PAPI_TOT_INS", data_point.papi_tot_ins},
        {"PAPI_L2_DCM", data_point.papi_l2_dcm}
    };
    
    // Update buffers, compute z-scores, and detect trends
    z_scores.papi_tot_cyc = normalizers["PAPI_TOT_CYC"].update(data["PAPI_TOT_CYC"]);
    z_scores.papi_tot_ins = normalizers["PAPI_TOT_INS"].update(data["PAPI_TOT_INS"]);
    z_scores.papi_l2_dcm = normalizers["PAPI_L2_DCM"].update(data["PAPI_L2_DCM"]);
    
    buffers["PAPI_TOT_CYC"].push_back(data["PAPI_TOT_CYC"]);
    buffers["PAPI_TOT_INS"].push_back(data["PAPI_TOT_INS"]);
    buffers["PAPI_L2_DCM"].push_back(data["PAPI_L2_DCM"]);
    
    trends.papi_tot_cyc = detect_trend("PAPI_TOT_CYC");
    trends.papi_tot_ins = detect_trend("PAPI_TOT_INS");
    trends.papi_l2_dcm = detect_trend("PAPI_L2_DCM");
    
    z_scores.timestamp = data_point.timestamp;
    z_scores.frequency = data_point.frequency;
    
    // Detect phase transition
    double max_z = std::max({std::abs(z_scores.papi_tot_cyc),
                             std::abs(z_scores.papi_tot_ins),
                             std::abs(z_scores.papi_l2_dcm)});
    bool is_transition = max_z > threshold;
    
    return {is_transition, z_scores, trends};
}