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

    /**
     * @brief Exports an ultra-crisp standalone SVG vector transit map.
     * @param filename Output SVG filepath (defaults to city_transit_network.svg)
     * @param highlightRoute Optional route result to highlight with a glowing neon trail
     * @param showCongestion If true, colors edges based on BPR V/C ratio
     * @param title Custom title header rendered on the map
     */
    bool exportSVG(const std::string& filename = "city_transit_network.svg",
                   const RouteResult* highlightRoute = nullptr,
                   bool showCongestion = false,
                   const std::string& title = "Colombo Smart City Public Transit Network") const;

    /**
     * @brief Exports a Graphviz DOT definition for graph rendering tools.
     * @param filename Output DOT filepath (defaults to transit_network.dot)
     * @param highlightRoute Optional route result to highlight
     */
    bool exportDOT(const std::string& filename = "transit_network.dot",
                   const RouteResult* highlightRoute = nullptr) const;
};

#endif // VISUALIZER_H
