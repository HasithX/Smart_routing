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

    RouteResult planJourney(const std::string& originId,
                            const std::string& destinationId,
                            RoutingPreference preference = RoutingPreference::FASTEST_TIME) const;

    std::string formatItinerary(const RouteResult& result) const;
    std::string comparePreferences(const std::string& originId, const std::string& destinationId) const;
};

#endif // ROUTEPLANNER_H
