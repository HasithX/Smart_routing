#include "Dijkstra.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <queue>

using namespace std;

string routingPreferenceToString(RoutingPreference pref) {
    switch (pref) {
        case RoutingPreference::MINIMUM_TRANSFERS:
            return "Minimum Transfers";
        case RoutingPreference::FASTEST_TIME:
        default:
            return "Fastest Travel Time";
    }
}

namespace {
    struct PQElement {
        double cost;
        string u;
        int mode; // -1 = start, 0 = bus, 1 = train

        bool operator>(const PQElement& other) const {
            return cost > other.cost;
        }
    };

    struct PredecessorRecord {
        string prevU;
        int prevMode;
        Edge edgeUsed;
        bool isTransfer;
        double transferDelay;
    };
} // namespace

RouteResult MultiModalDijkstra::findShortestPath(const string& originId,
                                                 const string& destinationId,
                                                 double customTransferPenalty) const {
    RouteResult result;
    result.origin = originId;
    result.destination = destinationId;
    result.preference = RoutingPreference::FASTEST_TIME;

    double penalty = (customTransferPenalty >= 0.0) ? customTransferPenalty : defaultTransferPenaltyMin;

    if (graph.nodes.find(originId) == graph.nodes.end() ||
        graph.nodes.find(destinationId) == graph.nodes.end()) {
        return result;
    }

    if (originId == destinationId) {
        result.pathNodes.push_back(originId);
        return result;
    }

    priority_queue<PQElement, vector<PQElement>, greater<PQElement>> pq;
    map<pair<string, int>, double> distances;
    map<pair<string, int>, PredecessorRecord> predecessors;

    pair<string, int> startState = {originId, -1};
    distances[startState] = 0.0;
    pq.push({0.0, originId, -1});

    pair<string, int> bestTerminalState = {"", -2};
    double minDestCost = numeric_limits<double>::infinity();

    while (!pq.empty()) {
        PQElement current = pq.top();
        pq.pop();

        pair<string, int> currState = {current.u, current.mode};

        auto itDist = distances.find(currState);
        if (itDist != distances.end() && current.cost > itDist->second) {
            continue;
        }

        if (current.u == destinationId) {
            if (current.cost < minDestCost) {
                minDestCost = current.cost;
                bestTerminalState = currState;
            }
            continue;
        }

        for (const auto& edge : graph.getNeighbors(current.u)) {
            int edgeMode = (edge.mode == TransportMode::TRAIN) ? 1 : 0;
            pair<string, int> nextState = {edge.target, edgeMode};

            bool isTransfer = false;
            double transferDelay = 0.0;

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
        result.totalTimeMin = numeric_limits<double>::infinity();
        return result;
    }

    // reconstruct path
    vector<RouteLeg> legs;
    vector<string> pathNodes;
    pathNodes.push_back(destinationId);

    pair<string, int> currState = bestTerminalState;
    double totalDistance = 0.0;
    int transferCount = 0;
    double totalPenalty = 0.0;

    while (predecessors.find(currState) != predecessors.end()) {
        const auto& rec = predecessors[currState];
        if (rec.isTransfer) {
            transferCount++;
            totalPenalty += rec.transferDelay;
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

    reverse(legs.begin(), legs.end());
    reverse(pathNodes.begin(), pathNodes.end());

    result.totalTimeMin = round(minDestCost * 100.0) / 100.0;
    result.totalDistanceKm = round(totalDistance * 100.0) / 100.0;
    result.transferCount = transferCount;
    result.totalTransferPenaltyMin = totalPenalty;
    result.legs = move(legs);
    result.pathNodes = move(pathNodes);

    return result;
}

RouteResult MultiModalDijkstra::findOptimalPath(const string& originId,
                                                const string& destinationId,
                                                RoutingPreference preference) const {
    double penalty = (preference == RoutingPreference::MINIMUM_TRANSFERS) ? 35.0 : defaultTransferPenaltyMin;
    RouteResult res = findShortestPath(originId, destinationId, penalty);
    res.preference = preference;
    return res;
}
