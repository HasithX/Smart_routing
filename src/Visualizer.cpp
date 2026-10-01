#include "Visualizer.h"
#include <fstream>
#include <iostream>
#include <set>

bool TransitVisualizer::exportSVG(const std::string& filename, const RouteResult* highlightRoute) const {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    // SVG Canvas: 1000 x 800 with dark modern theme (#0F172A)
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 1100 850\" width=\"100%\" height=\"100%\">\n";
    out << "  <rect width=\"1100\" height=\"850\" fill=\"#0F172A\" />\n";

    // Coordinate mapping: input coords x in [0, 100], y in [0, 100]
    // Map to canvas: cx = 100 + x * 9.0, cy = 750 - y * 6.5
    auto mapX = [](double x) { return 100.0 + x * 9.0; };
    auto mapY = [](double y) { return 770.0 - y * 6.8; };

    // Set of edges in highlight route
    std::set<std::pair<std::string, std::string>> highlightedEdges;
    if (highlightRoute && highlightRoute->isReachable()) {
        for (const auto& leg : highlightRoute->legs) {
            highlightedEdges.insert({leg.source, leg.target});
        }
    }

    out << "  <!-- Defs & Markers -->\n";
    out << "  <defs>\n";
    out << "    <marker id=\"arrow-green\" viewBox=\"0 0 10 10\" refX=\"6\" refY=\"5\" markerWidth=\"6\" markerHeight=\"6\" orient=\"auto-start-reverse\">\n";
    out << "      <path d=\"M 0 1 L 10 5 L 0 9 z\" fill=\"#10B981\" />\n";
    out << "    </marker>\n";
    out << "  </defs>\n";

    // 1. Draw Edges
    out << "  <!-- Transit Edges -->\n";
    for (const auto& pair : graph.adjList) {
        for (const auto& edge : pair.second) {
            const Location* srcLoc = graph.getNode(edge.source);
            const Location* tgtLoc = graph.getNode(edge.target);
            if (!srcLoc || !tgtLoc) continue;

            double x1 = mapX(srcLoc->x);
            double y1 = mapY(srcLoc->y);
            double x2 = mapX(tgtLoc->x);
            double y2 = mapY(tgtLoc->y);

            bool isHighlighted = highlightedEdges.count({edge.source, edge.target}) > 0;

            if (isHighlighted) {
                // Highlighted route in thick Emerald Green
                out << "  <line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2
                    << "\" stroke=\"#10B981\" stroke-width=\"5\" marker-end=\"url(#arrow-green)\" />\n";
            } else if (edge.mode == TransportMode::TRAIN) {
                // Train track: thick Crimson line
                out << "  <line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2
                    << "\" stroke=\"#EF4444\" stroke-width=\"3.5\" stroke-linecap=\"round\" opacity=\"0.85\" />\n";
            } else {
                // Bus route: dashed Blue line
                out << "  <line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2
                    << "\" stroke=\"#60A5FA\" stroke-width=\"1.8\" stroke-dasharray=\"6,4\" opacity=\"0.6\" />\n";
            }
        }
    }

    // 2. Draw Nodes
    out << "  <!-- Transit Stations & Hubs -->\n";
    for (const auto& pair : graph.nodes) {
        const Location& loc = pair.second;
        double cx = mapX(loc.x);
        double cy = mapY(loc.y);

        if (loc.isInterchange()) {
            // Interchange Hub: Gold Diamond
            out << "  <polygon points=\""
                << cx << "," << (cy - 12) << " "
                << (cx + 12) << "," << cy << " "
                << cx << "," << (cy + 12) << " "
                << (cx - 12) << "," << cy << "\" "
                << "fill=\"#F59E0B\" stroke=\"#FFFFFF\" stroke-width=\"2\" />\n";
        } else if (loc.nodeType == NodeType::TRAIN_STATION) {
            // Train Station: Red Square
            out << "  <rect x=\"" << (cx - 8) << "\" y=\"" << (cy - 8)
                << "\" width=\"16\" height=\"16\" fill=\"#EF4444\" stroke=\"#FFFFFF\" stroke-width=\"1.5\" />\n";
        } else {
            // Bus Stop: Blue Circle
            out << "  <circle cx=\"" << cx << "\" cy=\"" << cy
                << "\" r=\"6\" fill=\"#3B82F6\" stroke=\"#FFFFFF\" stroke-width=\"1.5\" />\n";
        }

        // Label
        out << "  <text x=\"" << (cx + 12) << "\" y=\"" << (cy + 4)
            << "\" fill=\"#F8FAFC\" font-family=\"sans-serif\" font-size=\"11\" font-weight=\"bold\">"
            << loc.id << ": " << loc.name << "</text>\n";
    }

    // 3. Legend & Header
    out << "  <!-- Title & Legend -->\n";
    out << "  <text x=\"50\" y=\"45\" fill=\"#FFFFFF\" font-family=\"sans-serif\" font-size=\"20\" font-weight=\"bold\">Smart City Multi-Modal Public Transit Network (Colombo Metro)</text>\n";
    out << "  <text x=\"50\" y=\"72\" fill=\"#94A3B8\" font-family=\"sans-serif\" font-size=\"12\">Advanced Data Structures &amp; Algorithms | C++17 Multi-Modal Graph</text>\n";

    out << "  <line x1=\"650\" y1=\"40\" x2=\"690\" y2=\"40\" stroke=\"#EF4444\" stroke-width=\"3.5\" />\n";
    out << "  <text x=\"700\" y=\"44\" fill=\"#F8FAFC\" font-family=\"sans-serif\" font-size=\"12\">Train Tracks (Solid Red)</text>\n";

    out << "  <line x1=\"650\" y1=\"65\" x2=\"690\" y2=\"65\" stroke=\"#60A5FA\" stroke-width=\"2\" stroke-dasharray=\"5,3\" />\n";
    out << "  <text x=\"700\" y=\"69\" fill=\"#F8FAFC\" font-family=\"sans-serif\" font-size=\"12\">Bus Routes (Dashed Blue)</text>\n";

    out << "  <polygon points=\"890,35 898,42 890,49 882,42\" fill=\"#F59E0B\" stroke=\"#FFFFFF\" stroke-width=\"1.5\" />\n";
    out << "  <text x=\"910\" y=\"44\" fill=\"#F8FAFC\" font-family=\"sans-serif\" font-size=\"12\">Interchange Hub</text>\n";

    if (highlightRoute && highlightRoute->isReachable()) {
        out << "  <line x1=\"850\" y1=\"65\" x2=\"890\" y2=\"65\" stroke=\"#10B981\" stroke-width=\"4\" />\n";
        out << "  <text x=\"900\" y=\"69\" fill=\"#10B981\" font-family=\"sans-serif\" font-size=\"12\">Optimal Route Path</text>\n";
    }

    out << "</svg>\n";
    out.close();
    return true;
}

bool TransitVisualizer::exportDOT(const std::string& filename, const RouteResult* highlightRoute) const {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    out << "digraph SmartTransitNetwork {\n";
    out << "  graph [bgcolor=\"#0F172A\", rankdir=LR];\n";
    out << "  node [fontname=\"Helvetica\", fontcolor=white, style=filled, penwidth=2];\n";
    out << "  edge [fontname=\"Helvetica\", fontsize=10, fontcolor=\"#94A3B8\"];\n\n";

    for (const auto& pair : graph.nodes) {
        const Location& loc = pair.second;
        std::string color = loc.isInterchange() ? "#F59E0B" : (loc.nodeType == NodeType::TRAIN_STATION ? "#EF4444" : "#3B82F6");
        std::string shape = loc.isInterchange() ? "diamond" : (loc.nodeType == NodeType::TRAIN_STATION ? "box" : "circle");
        out << "  \"" << loc.id << "\" [label=\"" << loc.id << "\\n" << loc.name << "\", fillcolor=\"" << color << "\", shape=" << shape << "];\n";
    }
    out << "\n";

    std::set<std::pair<std::string, std::string>> highlighted;
    if (highlightRoute && highlightRoute->isReachable()) {
        for (const auto& leg : highlightRoute->legs) {
            highlighted.insert({leg.source, leg.target});
        }
    }

    for (const auto& pair : graph.adjList) {
        for (const auto& edge : pair.second) {
            bool isHigh = highlighted.count({edge.source, edge.target}) > 0;
            std::string color = isHigh ? "#10B981" : (edge.mode == TransportMode::TRAIN ? "#EF4444" : "#60A5FA");
            std::string style = (edge.mode == TransportMode::TRAIN) ? "solid" : "dashed";
            int penwidth = isHigh ? 4 : (edge.mode == TransportMode::TRAIN ? 3 : 1);

            out << "  \"" << edge.source << "\" -> \"" << edge.target << "\" [color=\"" << color
                << "\", style=" << style << ", penwidth=" << penwidth
                << ", label=\"" << edge.baseTimeMin << "m\"];\n";
        }
    }

    out << "}\n";
    out.close();
    return true;
}
