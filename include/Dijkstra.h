#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "Graph.h"
#include <vector>
#include <string>

struct RouteLeg {
    std::string source;
    std::string target;
    TransportMode mode;
    double distanceKm;
    double travelTimeMin;
    bool isTransferBefore;
    double transferPenaltyMin;

    RouteLeg(std::string src, std::string tgt, TransportMode m, double dist,
             double time, bool isTrans = false, double penalty = 0.0)
        : source(std::move(src)), target(std::move(tgt)), mode(m), distanceKm(dist),
          travelTimeMin(time), isTransferBefore(isTrans), transferPenaltyMin(penalty) {}
};

struct RouteResult {
    std::string origin;
    std::string destination;
    double totalTimeMin;
    double totalDistanceKm;
    int transferCount;
    std::vector<RouteLeg> legs;
    std::vector<std::string> pathNodes;

    RouteResult() : totalTimeMin(0.0), totalDistanceKm(0.0), transferCount(0) {}
    bool isReachable() const { return !pathNodes.empty(); }
};

class MultiModalDijkstra {
private:
    const MultiModalGraph& graph;
    double defaultTransferPenaltyMin;

public:
    explicit MultiModalDijkstra(const MultiModalGraph& g, double defaultPenalty = 6.0)
        : graph(g), defaultTransferPenaltyMin(defaultPenalty) {}

    RouteResult findShortestPath(const std::string& originId,
                                 const std::string& destinationId,
                                 double customTransferPenalty = -1.0) const;
};

#endif // DIJKSTRA_H
