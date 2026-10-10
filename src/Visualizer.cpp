#include "Visualizer.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <set>

using namespace std;

namespace {
    struct Point {
        double x;
        double y;
    };

    string escapeXml(const string& str) {
        string res;
        for (char c : str) {
            if (c == '&') res += "&amp;";
            else if (c == '<') res += "&lt;";
            else if (c == '>') res += "&gt;";
            else if (c == '\"') res += "&quot;";
            else if (c == '\'') res += "&apos;";
            else res += c;
        }
        return res;
    }
}

bool TransitVisualizer::exportSVG(const string& filename,
                                  const RouteResult* highlightRoute,
                                  bool showCongestion,
                                  const string& title) const {
    ofstream out(filename);
    if (!out.is_open()) return false;

    if (graph.nodes.empty()) return false;

    // coordinate normalization
    double minX = 1e9, maxX = -1e9;
    double minY = 1e9, maxY = -1e9;
    for (const auto& pair : graph.nodes) {
        minX = min(minX, pair.second.x);
        maxX = max(maxX, pair.second.x);
        minY = min(minY, pair.second.y);
        maxY = max(maxY, pair.second.y);
    }

    if (abs(maxX - minX) < 1e-4) maxX = minX + 1.0;
    if (abs(maxY - minY) < 1e-4) maxY = minY + 1.0;

    const double canvasW = 1200.0;
    const double canvasH = 880.0;
    const double padX = 110.0;
    const double padY = 120.0;
    const double drawW = canvasW - 2.0 * padX;
    const double drawH = canvasH - 2.0 * padY - 40.0;

    auto toScreen = [&](double x, double y) -> Point {
        double nx = (x - minX) / (maxX - minX);
        double ny = (y - minY) / (maxY - minY);
        double sx = padX + nx * drawW;
        double sy = (canvasH - padY) - ny * drawH; // invert Y
        return {sx, sy};
    };

    set<pair<string, string>> highlightedEdges;
    if (highlightRoute && highlightRoute->isReachable()) {
        for (const auto& leg : highlightRoute->legs) {
            highlightedEdges.insert({leg.source, leg.target});
        }
    }

    // svg setup
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 "
        << canvasW << " " << canvasH << "\" width=\"100%\" height=\"100%\">\n";
    out << "  <rect width=\"100%\" height=\"100%\" fill=\"#0F172A\" />\n";

    // grid lines
    for (int gx = 100; gx < canvasW; gx += 100) {
        out << "  <line x1=\"" << gx << "\" y1=\"0\" x2=\"" << gx << "\" y2=\"" << canvasH
            << "\" stroke=\"#1E293B\" stroke-width=\"1\" stroke-dasharray=\"3,3\" />\n";
    }
    for (int gy = 100; gy < canvasH; gy += 100) {
        out << "  <line x1=\"0\" y1=\"" << gy << "\" x2=\"" << canvasW << "\" y2=\"" << gy
            << "\" stroke=\"#1E293B\" stroke-width=\"1\" stroke-dasharray=\"3,3\" />\n";
    }

    // markers and filters
    out << "  <defs>\n";
    out << "    <filter id=\"neonGlow\" x=\"-50%\" y=\"-50%\" width=\"200%\" height=\"200%\">\n";
    out << "      <feGaussianBlur stdDeviation=\"4\" result=\"blur\" />\n";
    out << "      <feMerge>\n";
    out << "        <feMergeNode in=\"blur\" />\n";
    out << "        <feMergeNode in=\"SourceGraphic\" />\n";
    out << "      </feMerge>\n";
    out << "    </filter>\n";
    out << "    <marker id=\"arrow-train\" viewBox=\"0 0 10 10\" refX=\"18\" refY=\"5\" markerWidth=\"5\" markerHeight=\"5\" orient=\"auto-start-reverse\">\n";
    out << "      <path d=\"M 0 1 L 10 5 L 0 9 z\" fill=\"#EF4444\" />\n";
    out << "    </marker>\n";
    out << "    <marker id=\"arrow-bus\" viewBox=\"0 0 10 10\" refX=\"16\" refY=\"5\" markerWidth=\"5\" markerHeight=\"5\" orient=\"auto-start-reverse\">\n";
    out << "      <path d=\"M 0 1 L 10 5 L 0 9 z\" fill=\"#60A5FA\" />\n";
    out << "    </marker>\n";
    out << "    <marker id=\"arrow-neon\" viewBox=\"0 0 10 10\" refX=\"20\" refY=\"5\" markerWidth=\"6\" markerHeight=\"6\" orient=\"auto-start-reverse\">\n";
    out << "      <path d=\"M 0 1 L 10 5 L 0 9 z\" fill=\"#10B981\" />\n";
    out << "    </marker>\n";
    out << "  </defs>\n\n";

    // draw edges
    out << "  <!-- Transit Routes / Edges -->\n";
    for (const auto& pair : graph.adjList) {
        for (const auto& edge : pair.second) {
            const Location* srcLoc = graph.getNode(edge.source);
            const Location* tgtLoc = graph.getNode(edge.target);
            if (!srcLoc || !tgtLoc) continue;

            Point p1 = toScreen(srcLoc->x, srcLoc->y);
            Point p2 = toScreen(tgtLoc->x, tgtLoc->y);

            double dx = p2.x - p1.x;
            double dy = p2.y - p1.y;
            double len = hypot(dx, dy);
            double offX = (len > 1e-4) ? (-dy / len * 3.0) : 0.0;
            double offY = (len > 1e-4) ? (dx / len * 3.0) : 0.0;

            double x1 = p1.x + offX, y1 = p1.y + offY;
            double x2 = p2.x + offX, y2 = p2.y + offY;

            string strokeColor = (edge.mode == TransportMode::TRAIN) ? "#EF4444" : "#60A5FA";
            string strokeDash = (edge.mode == TransportMode::TRAIN) ? "none" : "6,4";
            double strokeWidth = (edge.mode == TransportMode::TRAIN) ? 3.5 : 2.0;

            if (showCongestion) {
                double vc = (edge.capacity > 0) ? (static_cast<double>(edge.currentFlow) / edge.capacity) : 0.0;
                if (vc < 0.6) strokeColor = "#10B981";
                else if (vc < 0.85) strokeColor = "#F59E0B";
                else strokeColor = "#EF4444";
                strokeDash = "none";
                strokeWidth = (edge.mode == TransportMode::TRAIN) ? 4.0 : 3.0;
            }

            out << "  <line x1=\"" << fixed << setprecision(1) << x1
                << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2
                << "\" stroke=\"" << strokeColor << "\" stroke-width=\"" << strokeWidth
                << "\" stroke-dasharray=\"" << strokeDash << "\" stroke-linecap=\"round\" opacity=\"0.85\">\n";
            out << "    <title>Route: " << edge.routeId << " (" << transportModeToString(edge.mode) << ")\n"
                << escapeXml(srcLoc->name) << " -> " << escapeXml(tgtLoc->name) << "\n"
                << "Distance: " << edge.distanceKm << " km | Base Time: " << edge.baseTimeMin << " min\n"
                << "Congested Time: " << fixed << setprecision(1) << edge.getEffectiveTravelTime() << " min\n"
                << "Flow / Capacity: " << edge.currentFlow << " / " << edge.capacity << "</title>\n";
            out << "  </line>\n";
        }
    }

    // highlight optimal route
    if (highlightRoute && highlightRoute->isReachable()) {
        out << "  <!-- Highlighted Shortest Path Overlay -->\n";
        for (const auto& leg : highlightRoute->legs) {
            const Location* srcLoc = graph.getNode(leg.source);
            const Location* tgtLoc = graph.getNode(leg.target);
            if (!srcLoc || !tgtLoc) continue;

            Point p1 = toScreen(srcLoc->x, srcLoc->y);
            Point p2 = toScreen(tgtLoc->x, tgtLoc->y);

            out << "  <line x1=\"" << p1.x << "\" y1=\"" << p1.y
                << "\" x2=\"" << p2.x << "\" y2=\"" << p2.y
                << "\" stroke=\"#10B981\" stroke-width=\"5.5\" stroke-linecap=\"round\" "
                << "filter=\"url(#neonGlow)\" marker-end=\"url(#arrow-neon)\">\n";
            out << "    <title>[ACTIVE ROUTE] " << leg.source << " -> " << leg.target
                << " via " << transportModeToString(leg.mode) << " (" << leg.travelTimeMin << " min)</title>\n";
            out << "  </line>\n";
        }
    }

    // draw stations
    out << "  <!-- Stations and Hubs -->\n";
    for (const auto& pair : graph.nodes) {
        const Location& loc = pair.second;
        Point p = toScreen(loc.x, loc.y);

        out << "  <g transform=\"translate(" << p.x << "," << p.y << ")\">\n";

        if (loc.isInterchange()) {
            out << "    <circle r=\"16\" fill=\"#F59E0B\" opacity=\"0.2\" />\n";
            out << "    <polygon points=\"0,-12 12,0 0,12 -12,0\" fill=\"#F59E0B\" stroke=\"#FFFFFF\" stroke-width=\"2\" />\n";
        } else if (loc.nodeType == NodeType::TRAIN_STATION) {
            out << "    <rect x=\"-8\" y=\"-8\" width=\"16\" height=\"16\" rx=\"2\" fill=\"#EF4444\" stroke=\"#FFFFFF\" stroke-width=\"1.5\" />\n";
        } else {
            out << "    <circle r=\"6\" fill=\"#3B82F6\" stroke=\"#FFFFFF\" stroke-width=\"1.5\" />\n";
        }

        double badgeW = loc.name.length() * 6.5 + 16.0;
        out << "    <rect x=\"8\" y=\"-12\" width=\"" << badgeW
            << "\" height=\"16\" rx=\"3\" fill=\"#0F172A\" fill-opacity=\"0.85\" stroke=\"#334155\" stroke-width=\"1\" />\n";
        out << "    <text x=\"14\" y=\"0\" fill=\"#F8FAFC\" font-family=\"'Segoe UI', sans-serif\" font-size=\"10.5\" font-weight=\"bold\">"
            << loc.id << ": " << escapeXml(loc.name) << "</text>\n";

        out << "    <title>" << loc.id << ": " << escapeXml(loc.name) << "\n"
            << "Type: " << nodeTypeToString(loc.nodeType) << " | Zone: " << loc.zone << "</title>\n";
        out << "  </g>\n";
    }

    // stats card
    GraphStats stats = graph.getStats();
    double density = (stats.vertexCount > 1) ? (static_cast<double>(stats.totalEdges) / (stats.vertexCount * (stats.vertexCount - 1))) : 0.0;

    out << "  <!-- HUD Banner -->\n";
    out << "  <text x=\"45\" y=\"42\" fill=\"#FFFFFF\" font-family=\"'Segoe UI', sans-serif\" font-size=\"20\" font-weight=\"bold\">"
        << escapeXml(title) << "</text>\n";
    out << "  <text x=\"45\" y=\"66\" fill=\"#94A3B8\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11.5\">"
        << "Multi-Modal Public Transit Network | Adjacency List O(V + E)</text>\n";

    out << "  <g transform=\"translate(" << (canvasW - 250) << ", 25)\">\n";
    out << "    <rect width=\"215\" height=\"80\" rx=\"8\" fill=\"#1E293B\" opacity=\"0.9\" stroke=\"#334155\" />\n";
    out << "    <text x=\"12\" y=\"20\" fill=\"#F8FAFC\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11\" font-weight=\"bold\">GRAPH TOPOLOGY</text>\n";
    out << "    <text x=\"12\" y=\"38\" fill=\"#94A3B8\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11\">Stations |V|: <tspan fill=\"#38BDF8\" font-weight=\"bold\">" << stats.vertexCount << "</tspan></text>\n";
    out << "    <text x=\"12\" y=\"54\" fill=\"#94A3B8\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11\">Routes |E|: <tspan fill=\"#38BDF8\" font-weight=\"bold\">" << stats.totalEdges << "</tspan></text>\n";
    out << "    <text x=\"12\" y=\"70\" fill=\"#94A3B8\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11\">Density: <tspan fill=\"#38BDF8\" font-weight=\"bold\">"
        << fixed << setprecision(3) << density << " (Sparse)</tspan></text>\n";
    out << "  </g>\n";

    // legend
    out << "  <!-- Legend -->\n";
    out << "  <g transform=\"translate(45, " << (canvasH - 125) << ")\">\n";
    out << "    <rect width=\"360\" height=\"105\" rx=\"8\" fill=\"#1E293B\" opacity=\"0.9\" stroke=\"#334155\" />\n";
    out << "    <text x=\"12\" y=\"18\" fill=\"#F8FAFC\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11\" font-weight=\"bold\">MAP LEGEND</text>\n";
    out << "    <line x1=\"12\" y1=\"35\" x2=\"45\" y2=\"35\" stroke=\"#EF4444\" stroke-width=\"3\" />\n";
    out << "    <text x=\"55\" y=\"38\" fill=\"#F8FAFC\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11\">Train Tracks (Dedicated Rail)</text>\n";
    out << "    <line x1=\"12\" y1=\"55\" x2=\"45\" y2=\"55\" stroke=\"#60A5FA\" stroke-width=\"2\" stroke-dasharray=\"5,3\" />\n";
    out << "    <text x=\"55\" y=\"58\" fill=\"#F8FAFC\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11\">Bus Corridors (Roadways)</text>\n";

    if (highlightRoute && highlightRoute->isReachable()) {
        out << "    <line x1=\"12\" y1=\"75\" x2=\"45\" y2=\"75\" stroke=\"#10B981\" stroke-width=\"4\" filter=\"url(#neonGlow)\" />\n";
        out << "    <text x=\"55\" y=\"78\" fill=\"#10B981\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11\" font-weight=\"bold\">Optimal Shortest Path ("
            << fixed << setprecision(1) << highlightRoute->totalTimeMin << " min)</text>\n";
    }

    out << "    <polygon points=\"240,30 248,37 240,44 232,37\" fill=\"#F59E0B\" stroke=\"#FFFFFF\" stroke-width=\"1.5\" />\n";
    out << "    <text x=\"256\" y=\"40\" fill=\"#F8FAFC\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11\">Hub</text>\n";
    out << "    <rect x=\"234\" y=\"50\" width=\"12\" height=\"12\" fill=\"#EF4444\" stroke=\"#FFFFFF\" stroke-width=\"1\" />\n";
    out << "    <text x=\"256\" y=\"60\" fill=\"#F8FAFC\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11\">Train Stn</text>\n";
    out << "    <circle cx=\"240\" cy=\"78\" r=\"5\" fill=\"#3B82F6\" stroke=\"#FFFFFF\" stroke-width=\"1\" />\n";
    out << "    <text x=\"256\" y=\"82\" fill=\"#F8FAFC\" font-family=\"'Segoe UI', sans-serif\" font-size=\"11\">Bus Stop</text>\n";
    out << "  </g>\n";

    out << "</svg>\n";
    out.close();
    return true;
}

bool TransitVisualizer::exportDOT(const string& filename, const RouteResult* highlightRoute) const {
    ofstream out(filename);
    if (!out.is_open()) return false;

    out << "digraph SmartTransitNetwork {\n";
    out << "  graph [bgcolor=\"#0F172A\", rankdir=LR];\n";
    out << "  node [fontname=\"Segoe UI\", fontcolor=white, style=filled, penwidth=2];\n";
    out << "  edge [fontname=\"Segoe UI\", fontsize=10, fontcolor=\"#94A3B8\"];\n\n";

    for (const auto& pair : graph.nodes) {
        const Location& loc = pair.second;
        string color = loc.isInterchange() ? "#F59E0B" : (loc.nodeType == NodeType::TRAIN_STATION ? "#EF4444" : "#3B82F6");
        string shape = loc.isInterchange() ? "doubleoctagon" : (loc.nodeType == NodeType::TRAIN_STATION ? "box" : "circle");
        out << "  \"" << loc.id << "\" [label=\"" << loc.id << "\\n" << loc.name << "\", fillcolor=\"" << color << "\", shape=" << shape << "];\n";
    }
    out << "\n";

    set<pair<string, string>> highlighted;
    if (highlightRoute && highlightRoute->isReachable()) {
        for (const auto& leg : highlightRoute->legs) {
            highlighted.insert({leg.source, leg.target});
        }
    }

    for (const auto& pair : graph.adjList) {
        for (const auto& edge : pair.second) {
            bool isHigh = highlighted.count({edge.source, edge.target}) > 0;
            string color = isHigh ? "#10B981" : (edge.mode == TransportMode::TRAIN ? "#EF4444" : "#60A5FA");
            string style = (edge.mode == TransportMode::TRAIN) ? "solid" : "dashed";
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
