#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "Graph.h"
#include <vector>
#include <string>

using namespace std;

enum class RoutingPreference {
    FASTEST_TIME,
    MINIMUM_TRANSFERS
};

string routingPreferenceToString(RoutingPreference pref);

struct RouteLeg {
    string source;
    string target;
    TransportMode mode;
    double distanceKm;
    double travelTimeMin;
    bool isTransferBefore;
    double transferPenaltyMin;

    RouteLeg(string src, string tgt, TransportMode m, double dist,
             double time, bool isTrans = false, double penalty = 0.0)
        : source(move(src)), target(move(tgt)), mode(m), distanceKm(dist),
          travelTimeMin(time), isTransferBefore(isTrans), transferPenaltyMin(penalty) {}
};

struct RouteResult {
    string origin;
    string destination;
    double totalTimeMin;
    double totalDistanceKm;
    int transferCount;
    double totalTransferPenaltyMin;
    RoutingPreference preference;
    vector<RouteLeg> legs;
    vector<string> pathNodes;

    RouteResult() : totalTimeMin(0.0), totalDistanceKm(0.0), transferCount(0),
                    totalTransferPenaltyMin(0.0), preference(RoutingPreference::FASTEST_TIME) {}
    bool isReachable() const { return !pathNodes.empty(); }
};

class MultiModalDijkstra {
private:
    const MultiModalGraph& graph;
    double defaultTransferPenaltyMin;

public:
    explicit MultiModalDijkstra(const MultiModalGraph& g, double defaultPenalty = 6.0)
        : graph(g), defaultTransferPenaltyMin(defaultPenalty) {}

    RouteResult findShortestPath(const string& originId,
                                 const string& destinationId,
                                 double customTransferPenalty = -1.0) const;

    RouteResult findOptimalPath(const string& originId,
                                const string& destinationId,
                                RoutingPreference preference) const;
};

#endif // DIJKSTRA_H
