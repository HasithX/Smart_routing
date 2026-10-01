#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "Graph.h"
#include "Dijkstra.h"
#include <string>

class TransitVisualizer {
private:
    const MultiModalGraph& graph;

public:
    explicit TransitVisualizer(const MultiModalGraph& g) : graph(g) {}

    bool exportSVG(const std::string& filename = "city_transit_network.svg",
                   const RouteResult* highlightRoute = nullptr) const;

    bool exportDOT(const std::string& filename = "city_transit_network.dot",
                   const RouteResult* highlightRoute = nullptr) const;
};

#endif // VISUALIZER_H
