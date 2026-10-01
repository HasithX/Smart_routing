#ifndef EDGE_H
#define EDGE_H

#include <string>

enum class TransportMode {
    BUS,
    TRAIN
};

std::string transportModeToString(TransportMode mode);
TransportMode stringToTransportMode(const std::string& str);

struct Edge {
    std::string routeId;
    std::string source;
    std::string target;
    TransportMode mode;
    double distanceKm;
    double baseTimeMin;
    int capacity;
    int currentFlow;

    Edge() : mode(TransportMode::BUS), distanceKm(0.0), baseTimeMin(0.0), capacity(100), currentFlow(0) {}
    Edge(std::string rId, std::string src, std::string tgt, TransportMode m,
         double dist, double baseTime, int cap = 100, int flow = 0)
        : routeId(std::move(rId)), source(std::move(src)), target(std::move(tgt)),
          mode(m), distanceKm(dist), baseTimeMin(baseTime), capacity(cap), currentFlow(flow) {}

    double getEffectiveTravelTime() const;
    void addFlow(int passengerCount = 1);
    void resetFlow();
};

#endif // EDGE_H
