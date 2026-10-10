#ifndef GRAPH_H
#define GRAPH_H

#include "Location.h"
#include "Edge.h"
#include <unordered_map>
#include <vector>
#include <string>

using namespace std;

struct GraphStats {
    int vertexCount;
    int totalEdges;
    int busEdges;
    int trainEdges;
    int interchangeHubs;
};

class MultiModalGraph {
public:
    unordered_map<string, Location> nodes;
    unordered_map<string, vector<Edge>> adjList;

    MultiModalGraph() = default;

    void addNode(const Location& node);
    void addEdge(const Edge& edge, bool bidirectional = true);
    
    const vector<Edge>& getNeighbors(const string& nodeId) const;
    const Location* getNode(const string& nodeId) const;

    bool loadFromCSV(const string& stationsPath, const string& routesPath);
    bool isConnected() const;
    void resetAllCongestion();
    GraphStats getStats() const;
};

#endif // GRAPH_H
