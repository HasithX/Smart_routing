#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "Graph.h"
#include "Dijkstra.h"
#include <string>

using namespace std;

class TransitVisualizer {
private:
    const MultiModalGraph& graph;

public:
    explicit TransitVisualizer(const MultiModalGraph& g) : graph(g) {}

    bool exportSVG(const string& filename = "city_transit_network.svg",
                   const RouteResult* highlightRoute = nullptr,
                   bool showCongestion = false,
                   const string& title = "Colombo Smart City Public Transit Network") const;

    bool exportDOT(const string& filename = "transit_network.dot",
                   const RouteResult* highlightRoute = nullptr) const;
};

#endif // VISUALIZER_H
