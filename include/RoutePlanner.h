#ifndef ROUTEPLANNER_H
#define ROUTEPLANNER_H

#include "Graph.h"
#include "Dijkstra.h"
#include <string>

using namespace std;

class RoutePlanner {
private:
    const MultiModalGraph& graph;
    MultiModalDijkstra router;

public:
    explicit RoutePlanner(const MultiModalGraph& g, double defaultTransferPenalty = 6.0)
        : graph(g), router(g, defaultTransferPenalty) {}

    RouteResult planJourney(const string& originId,
                            const string& destinationId,
                            RoutingPreference preference = RoutingPreference::FASTEST_TIME) const;

    string formatItinerary(const RouteResult& result) const;
    string comparePreferences(const string& originId, const string& destinationId) const;
};

#endif // ROUTEPLANNER_H
