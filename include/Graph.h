#ifndef GRAPH_H
#define GRAPH_H

#include "Location.h"
#include "Edge.h"
#include <unordered_map>
#include <vector>
#include <string>

struct GraphStats {
    int vertexCount;
    int totalEdges;
    int busEdges;
    int trainEdges;
    int interchangeHubs;
};

class MultiModalGraph {
public:
    // Vertices map: Node ID -> Location
    std::unordered_map<std::string, Location> nodes;

    // Adjacency List: Node ID -> List of Outgoing Edges
    // Space Complexity: O(V + E)
    std::unordered_map<std::string, std::vector<Edge>> adjList;

    MultiModalGraph() = default;

    void addNode(const Location& node);
    void addEdge(const Edge& edge, bool bidirectional = true);
    
    const std::vector<Edge>& getNeighbors(const std::string& nodeId) const;
    const Location* getNode(const std::string& nodeId) const;

    bool loadFromCSV(const std::string& stationsPath, const std::string& routesPath);
    bool isConnected() const;
    void resetAllCongestion();
    GraphStats getStats() const;
};

#endif // GRAPH_H
