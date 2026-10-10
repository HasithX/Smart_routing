#include "Edge.h"
#include <cmath>
#include <algorithm>

using namespace std;

string transportModeToString(TransportMode mode) {
    return (mode == TransportMode::TRAIN) ? "TRAIN" : "BUS";
}

TransportMode stringToTransportMode(const string& str) {
    string s = str;
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    if (s == "TRAIN") return TransportMode::TRAIN;
    return TransportMode::BUS;
}

double Edge::getEffectiveTravelTime() const {
    if (capacity <= 0) return baseTimeMin;

    double volumeToCapacity = static_cast<double>(currentFlow) / capacity;
    double alpha;
    double beta;

    if (mode == TransportMode::TRAIN) {
        alpha = 0.15;
        beta = 2.0;
    } else {
        alpha = 0.60;
        beta = 2.5;
    }

    double congestionMultiplier = 1.0 + alpha * pow(volumeToCapacity, beta);
    return round(baseTimeMin * congestionMultiplier * 100.0) / 100.0;
}

void Edge::addFlow(int passengerCount) {
    currentFlow += passengerCount;
}

void Edge::resetFlow() {
    currentFlow = 0;
}
