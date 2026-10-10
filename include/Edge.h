#ifndef EDGE_H
#define EDGE_H

#include <string>

using namespace std;

enum class TransportMode {
    BUS,
    TRAIN
};

string transportModeToString(TransportMode mode);
TransportMode stringToTransportMode(const string& str);

struct Edge {
    string routeId;
    string source;
    string target;
    TransportMode mode;
    double distanceKm;
    double baseTimeMin;
    int capacity;
    int currentFlow;

    Edge() : mode(TransportMode::BUS), distanceKm(0.0), baseTimeMin(0.0), capacity(100), currentFlow(0) {}
    Edge(string rId, string src, string tgt, TransportMode m,
         double dist, double baseTime, int cap = 100, int flow = 0)
        : routeId(move(rId)), source(move(src)), target(move(tgt)),
          mode(m), distanceKm(dist), baseTimeMin(baseTime), capacity(cap), currentFlow(flow) {}

    double getEffectiveTravelTime() const;
    void addFlow(int passengerCount = 1);
    void resetFlow();
};

#endif // EDGE_H
