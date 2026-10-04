#include "Edge.h"
#include <cmath>
#include <algorithm>

std::string transportModeToString(TransportMode mode) {
    return (mode == TransportMode::TRAIN) ? "TRAIN" : "BUS";
}

TransportMode stringToTransportMode(const std::string& str) {
    std::string s = str;
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    if (s == "TRAIN") return TransportMode::TRAIN;
    return TransportMode::BUS;
}

double Edge::getEffectiveTravelTime() const {
    if (capacity <= 0) return baseTimeMin;

    double volumeToCapacity = static_cast<double>(currentFlow) / capacity;

    double alpha;
    double beta;

    if (mode == TransportMode::TRAIN) {
        // Dedicated rail lines suffer minimal delay from passenger volume
        alpha = 0.15;
        beta = 2.0;
    } else {
        // Buses on roadways experience heavier congestion impedance
        alpha = 0.60;
        beta = 2.5;
    }

    double congestionMultiplier = 1.0 + alpha * std::pow(volumeToCapacity, beta);
    return std::round(baseTimeMin * congestionMultiplier * 100.0) / 100.0;
}

void Edge::addFlow(int passengerCount) {
    currentFlow += passengerCount;
}

void Edge::resetFlow() {
    currentFlow = 0;
}
