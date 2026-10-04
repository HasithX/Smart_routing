#ifndef ROUTEPLANNER_H
#define ROUTEPLANNER_H

#include "Graph.h"
#include "Dijkstra.h"
#include <string>

class RoutePlanner {
private:
    const MultiModalGraph& graph;
    MultiModalDijkstra router;

public:
    explicit RoutePlanner(const MultiModalGraph& g, double defaultTransferPenalty = 6.0)
        : graph(g), router(g, defaultTransferPenalty) {}

    RouteResult planJourney(const std::string& originId, const std::string& destinationId) const;
    std::string formatItinerary(const RouteResult& result) const;
};

#endif // ROUTEPLANNER_H
