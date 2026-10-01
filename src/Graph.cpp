#include "Graph.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace {
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }
}

void MultiModalGraph::addNode(const Location& node) {
    if (nodes.find(node.id) == nodes.end()) {
        nodes[node.id] = node;
        adjList[node.id] = std::vector<Edge>();
    }
}

void MultiModalGraph::addEdge(const Edge& edge, bool bidirectional) {
    if (nodes.find(edge.source) == nodes.end() || nodes.find(edge.target) == nodes.end()) {
        std::cerr << "Warning: Cannot add edge " << edge.routeId 
                  << ". Both endpoints must exist in graph." << std::endl;
        return;
    }

    adjList[edge.source].push_back(edge);

    if (bidirectional) {
        Edge revEdge(
            edge.routeId + "_REV",
            edge.target,
            edge.source,
            edge.mode,
            edge.distanceKm,
            edge.baseTimeMin,
            edge.capacity,
            edge.currentFlow
        );
        adjList[edge.target].push_back(revEdge);
    }
}

const std::vector<Edge>& MultiModalGraph::getNeighbors(const std::string& nodeId) const {
    static const std::vector<Edge> emptyList;
    auto it = adjList.find(nodeId);
    if (it != adjList.end()) {
        return it->second;
    }
    return emptyList;
}

const Location* MultiModalGraph::getNode(const std::string& nodeId) const {
    auto it = nodes.find(nodeId);
    if (it != nodes.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool MultiModalGraph::loadFromCSV(const std::string& stationsPath, const std::string& routesPath) {
    // 1. Load Stations
    std::ifstream stFile(stationsPath);
    if (!stFile.is_open()) {
        std::cerr << "Error: Could not open stations CSV at " << stationsPath << std::endl;
        return false;
    }

    std::string line;
    // Skip header: id,name,type,zone,x,y
    if (std::getline(stFile, line)) {
        while (std::getline(stFile, line)) {
            line = trim(line);
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string id, name, typeStr, zone, xStr, yStr;

            if (std::getline(ss, id, ',') &&
                std::getline(ss, name, ',') &&
                std::getline(ss, typeStr, ',') &&
                std::getline(ss, zone, ',') &&
                std::getline(ss, xStr, ',') &&
                std::getline(ss, yStr, ',')) {

                double x = std::stod(trim(xStr));
                double y = std::stod(trim(yStr));
                Location loc(trim(id), trim(name), stringToNodeType(trim(typeStr)), trim(zone), x, y);
                addNode(loc);
            }
        }
    }
    stFile.close();

    // 2. Load Routes
    std::ifstream rtFile(routesPath);
    if (!rtFile.is_open()) {
        std::cerr << "Error: Could not open routes CSV at " << routesPath << std::endl;
        return false;
    }

    // Skip header: route_id,source_id,target_id,mode,distance_km,base_time_min,capacity,bidirectional
    if (std::getline(rtFile, line)) {
        while (std::getline(rtFile, line)) {
            line = trim(line);
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string routeId, src, tgt, modeStr, distStr, timeStr, capStr, bidiStr;

            if (std::getline(ss, routeId, ',') &&
                std::getline(ss, src, ',') &&
                std::getline(ss, tgt, ',') &&
                std::getline(ss, modeStr, ',') &&
                std::getline(ss, distStr, ',') &&
                std::getline(ss, timeStr, ',') &&
                std::getline(ss, capStr, ',') &&
                std::getline(ss, bidiStr, ',')) {

                double dist = std::stod(trim(distStr));
                double time = std::stod(trim(timeStr));
                int cap = std::stoi(trim(capStr));
                std::string bidiClean = trim(bidiStr);
                std::transform(bidiClean.begin(), bidiClean.end(), bidiClean.begin(), ::tolower);
                bool bidi = (bidiClean == "true" || bidiClean == "1");

                Edge edge(trim(routeId), trim(src), trim(tgt), stringToTransportMode(trim(modeStr)), dist, time, cap);
                addEdge(edge, bidi);
            }
        }
    }
    rtFile.close();
    return true;
}

void MultiModalGraph::resetAllCongestion() {
    for (auto& pair : adjList) {
        for (auto& edge : pair.second) {
            edge.resetFlow();
        }
    }
}

GraphStats MultiModalGraph::getStats() const {
    GraphStats stats{0, 0, 0, 0, 0};
    stats.vertexCount = static_cast<int>(nodes.size());

    for (const auto& pair : nodes) {
        if (pair.second.isInterchange()) {
            stats.interchangeHubs++;
        }
    }

    for (const auto& pair : adjList) {
        stats.totalEdges += static_cast<int>(pair.second.size());
        for (const auto& edge : pair.second) {
            if (edge.mode == TransportMode::TRAIN) {
                stats.trainEdges++;
            } else {
                stats.busEdges++;
            }
        }
    }

    return stats;
}
