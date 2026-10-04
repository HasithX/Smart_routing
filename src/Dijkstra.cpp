#include "Dijkstra.h"
#include <queue>
#include <map>
#include <limits>
#include <algorithm>
#include <cmath>

namespace {
    struct PQElement {
        double cost;
        std::string u;
        int mode; // -1 = Start (no prior mode), 0 = BUS, 1 = TRAIN

        bool operator>(const PQElement& other) const {
            return cost > other.cost; // Min-Heap ordering
        }
    };

    struct PredecessorRecord {
        std::string prevU;
        int prevMode;
        Edge edgeUsed;
        bool isTransfer;
        double transferDelay;
    };
}

RouteResult MultiModalDijkstra::findShortestPath(const std::string& originId,
                                                 const std::string& destinationId,
                                                 double customTransferPenalty) const {
    RouteResult result;
    result.origin = originId;
    result.destination = destinationId;

    double penalty = (customTransferPenalty >= 0.0) ? customTransferPenalty : defaultTransferPenaltyMin;

    if (graph.nodes.find(originId) == graph.nodes.end() ||
        graph.nodes.find(destinationId) == graph.nodes.end()) {
        return result;
    }

    if (originId == destinationId) {
        result.pathNodes.push_back(originId);
        return result;
    }

    // Min-Heap Priority Queue
    std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> pq;

    // Distances map: State (NodeID, Mode) -> Min Cost
    std::map<std::pair<std::string, int>, double> distances;

    // Predecessors map: State -> Predecessor Record
    std::map<std::pair<std::string, int>, PredecessorRecord> predecessors;

    // Initial state: at origin with no prior transport mode (-1)
    std::pair<std::string, int> startState = {originId, -1};
    distances[startState] = 0.0;
    pq.push({0.0, originId, -1});

    std::pair<std::string, int> bestTerminalState = {"", -2};
    double minDestCost = std::numeric_limits<double>::infinity();

    while (!pq.empty()) {
        PQElement current = pq.top();
        pq.pop();

        std::pair<std::string, int> currState = {current.u, current.mode};

        // Pruning if a strictly lower cost path to this state was already processed
        auto itDist = distances.find(currState);
        if (itDist != distances.end() && current.cost > itDist->second) {
            continue;
        }

        // Check if destination reached
        if (current.u == destinationId) {
            if (current.cost < minDestCost) {
                minDestCost = current.cost;
                bestTerminalState = currState;
            }
            continue;
        }

        // Expand outgoing edges from adjacency list
        for (const auto& edge : graph.getNeighbors(current.u)) {
            int edgeMode = (edge.mode == TransportMode::TRAIN) ? 1 : 0;
            std::pair<std::string, int> nextState = {edge.target, edgeMode};

            bool isTransfer = false;
            double transferDelay = 0.0;

            // Transfer occurs when transitioning between two valid different modes
            if (current.mode != -1 && current.mode != edgeMode) {
                isTransfer = true;
                transferDelay = penalty;
            }

            double effectiveEdgeTime = edge.getEffectiveTravelTime();
            double newCost = current.cost + effectiveEdgeTime + transferDelay;

            auto itNext = distances.find(nextState);
            if (itNext == distances.end() || newCost < itNext->second) {
                distances[nextState] = newCost;
                predecessors[nextState] = PredecessorRecord{
                    current.u,
                    current.mode,
                    edge,
                    isTransfer,
                    transferDelay
                };
                pq.push({newCost, edge.target, edgeMode});
            }
        }
    }

    if (bestTerminalState.second == -2) {
        // Destination unreachable
        result.totalTimeMin = std::numeric_limits<double>::infinity();
        return result;
    }

    // Path & Leg Reconstruction in O(L)
    std::vector<RouteLeg> legs;
    std::vector<std::string> pathNodes;
    pathNodes.push_back(destinationId);

    std::pair<std::string, int> currState = bestTerminalState;
    double totalDistance = 0.0;
    int transferCount = 0;

    while (predecessors.find(currState) != predecessors.end()) {
        const auto& rec = predecessors[currState];
        if (rec.isTransfer) {
            transferCount++;
        }

        legs.emplace_back(
            rec.edgeUsed.source,
            rec.edgeUsed.target,
            rec.edgeUsed.mode,
            rec.edgeUsed.distanceKm,
            rec.edgeUsed.getEffectiveTravelTime(),
            rec.isTransfer,
            rec.transferDelay
        );

        totalDistance += rec.edgeUsed.distanceKm;
        pathNodes.push_back(rec.prevU);
        currState = {rec.prevU, rec.prevMode};
    }

    std::reverse(legs.begin(), legs.end());
    std::reverse(pathNodes.begin(), pathNodes.end());

    result.totalTimeMin = std::round(minDestCost * 100.0) / 100.0;
    result.totalDistanceKm = std::round(totalDistance * 100.0) / 100.0;
    result.transferCount = transferCount;
    result.legs = std::move(legs);
    result.pathNodes = std::move(pathNodes);

    return result;
}
