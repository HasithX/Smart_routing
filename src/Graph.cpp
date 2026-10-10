#include "Graph.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include <unordered_set>

using namespace std;

namespace {
    string trim(const string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }
}

void MultiModalGraph::addNode(const Location& node) {
    if (nodes.find(node.id) == nodes.end()) {
        nodes[node.id] = node;
        adjList[node.id] = vector<Edge>();
    }
}

void MultiModalGraph::addEdge(const Edge& edge, bool bidirectional) {
    if (nodes.find(edge.source) == nodes.end() || nodes.find(edge.target) == nodes.end()) {
        cerr << "Warning: Cannot add edge " << edge.routeId 
             << ". Both endpoints must exist in graph." << endl;
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

const vector<Edge>& MultiModalGraph::getNeighbors(const string& nodeId) const {
    static const vector<Edge> emptyList;
    auto it = adjList.find(nodeId);
    if (it != adjList.end()) {
        return it->second;
    }
    return emptyList;
}

const Location* MultiModalGraph::getNode(const string& nodeId) const {
    auto it = nodes.find(nodeId);
    if (it != nodes.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool MultiModalGraph::isConnected() const {
    if (nodes.empty()) {
        return false;
    }

    queue<string> stationsToVisit;
    unordered_set<string> visited;
    const string& startId = nodes.begin()->first;

    stationsToVisit.push(startId);
    visited.insert(startId);

    while (!stationsToVisit.empty()) {
        const string currentId = stationsToVisit.front();
        stationsToVisit.pop();

        for (const Edge& edge : getNeighbors(currentId)) {
            if (nodes.find(edge.target) != nodes.end() && visited.insert(edge.target).second) {
                stationsToVisit.push(edge.target);
            }
        }
    }

    return visited.size() == nodes.size();
}

bool MultiModalGraph::loadFromCSV(const string& stationsPath, const string& routesPath) {
    // load stations
    ifstream stFile(stationsPath);
    if (!stFile.is_open()) {
        cerr << "Error: Could not open stations CSV at " << stationsPath << endl;
        return false;
    }

    string line;
    if (getline(stFile, line)) {
        while (getline(stFile, line)) {
            line = trim(line);
            if (line.empty()) continue;
            stringstream ss(line);
            string id, name, typeStr, zone, xStr, yStr;

            if (getline(ss, id, ',') &&
                getline(ss, name, ',') &&
                getline(ss, typeStr, ',') &&
                getline(ss, zone, ',') &&
                getline(ss, xStr, ',') &&
                getline(ss, yStr, ',')) {

                double x = stod(trim(xStr));
                double y = stod(trim(yStr));
                Location loc(trim(id), trim(name), stringToNodeType(trim(typeStr)), trim(zone), x, y);
                addNode(loc);
            }
        }
    }
    stFile.close();

    // load routes
    ifstream rtFile(routesPath);
    if (!rtFile.is_open()) {
        cerr << "Error: Could not open routes CSV at " << routesPath << endl;
        return false;
    }

    if (getline(rtFile, line)) {
        while (getline(rtFile, line)) {
            line = trim(line);
            if (line.empty()) continue;
            stringstream ss(line);
            string routeId, src, tgt, modeStr, distStr, timeStr, capStr, bidiStr;

            if (getline(ss, routeId, ',') &&
                getline(ss, src, ',') &&
                getline(ss, tgt, ',') &&
                getline(ss, modeStr, ',') &&
                getline(ss, distStr, ',') &&
                getline(ss, timeStr, ',') &&
                getline(ss, capStr, ',') &&
                getline(ss, bidiStr, ',')) {

                double dist = stod(trim(distStr));
                double time = stod(trim(timeStr));
                int cap = stoi(trim(capStr));
                string bidiClean = trim(bidiStr);
                transform(bidiClean.begin(), bidiClean.end(), bidiClean.begin(), ::tolower);
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
