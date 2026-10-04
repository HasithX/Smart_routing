#include "DemandSimulator.h"
#include <random>
#include <algorithm>
#include <cmath>

std::string timeWindowToString(TimeWindow tw) {
    switch (tw) {
        case TimeWindow::MORNING_PEAK: return "07:00 - 09:30 AM (Morning Peak Commute)";
        case TimeWindow::MIDDAY_OFF_PEAK: return "11:00 AM - 02:00 PM (Midday Off-Peak)";
        case TimeWindow::EVENING_PEAK: return "04:30 - 07:30 PM (Evening Peak Commute)";
        case TimeWindow::NIGHT_LOW: return "10:00 PM - 04:00 AM (Night Low Demand)";
    }
    return "Standard Window";
}

std::vector<PassengerTrip> DemandSimulator::generateTrips(TimeWindow window, int totalPassengers, unsigned int seed) {
    std::vector<PassengerTrip> trips;
    std::mt19937 rng(seed);

    std::vector<std::string> allNodeIds;
    std::vector<std::string> residentialNodes;
    std::vector<std::string> commercialNodes;

    for (const auto& pair : graph.nodes) {
        allNodeIds.push_back(pair.first);
        std::string z = pair.second.zone;
        std::transform(z.begin(), z.end(), z.begin(), ::tolower);
        if (z == "residential") {
            residentialNodes.push_back(pair.first);
        } else if (z == "commercial" || z == "industrial" || z == "educational" || z == "administrative") {
            commercialNodes.push_back(pair.first);
        }
    }

    if (residentialNodes.empty()) residentialNodes = allNodeIds;
    if (commercialNodes.empty()) commercialNodes = allNodeIds;

    std::uniform_real_distribution<double> dist01(0.0, 1.0);
    std::uniform_int_distribution<int> randMin(0, 59);

    for (int i = 0; i < totalPassengers; ++i) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "PX_%04d", i + 1);
        std::string pId(buf);

        std::string orig;
        std::string dest;
        std::string depTime;

        if (window == TimeWindow::MORNING_PEAK) {
            // 80% flow from suburbs into city/commercial centers
            if (dist01(rng) < 0.80) {
                orig = residentialNodes[rng() % residentialNodes.size()];
                dest = commercialNodes[rng() % commercialNodes.size()];
            } else {
                orig = allNodeIds[rng() % allNodeIds.size()];
                dest = allNodeIds[rng() % allNodeIds.size()];
            }
            std::snprintf(buf, sizeof(buf), "07:%02d AM", randMin(rng));
            depTime = buf;
        } else if (window == TimeWindow::EVENING_PEAK) {
            // 80% reverse flow back to suburbs
            if (dist01(rng) < 0.80) {
                orig = commercialNodes[rng() % commercialNodes.size()];
                dest = residentialNodes[rng() % residentialNodes.size()];
            } else {
                orig = allNodeIds[rng() % allNodeIds.size()];
                dest = allNodeIds[rng() % allNodeIds.size()];
            }
            std::snprintf(buf, sizeof(buf), "05:%02d PM", randMin(rng));
            depTime = buf;
        } else if (window == TimeWindow::MIDDAY_OFF_PEAK) {
            orig = allNodeIds[rng() % allNodeIds.size()];
            dest = allNodeIds[rng() % allNodeIds.size()];
            std::snprintf(buf, sizeof(buf), "01:%02d PM", randMin(rng));
            depTime = buf;
        } else {
            orig = allNodeIds[rng() % allNodeIds.size()];
            dest = allNodeIds[rng() % allNodeIds.size()];
            std::snprintf(buf, sizeof(buf), "11:%02d PM", randMin(rng));
            depTime = buf;
        }

        while (orig == dest) {
            dest = allNodeIds[rng() % allNodeIds.size()];
        }

        trips.push_back({pId, orig, dest, depTime, window});
    }

    return trips;
}

SimulationResult DemandSimulator::simulateTimeWindow(TimeWindow window, int totalPassengers, bool applyFeedback) {
    graph.resetAllCongestion();

    auto trips = generateTrips(window, totalPassengers);
    MultiModalDijkstra router(graph);

    int routed = 0;
    int unreachable = 0;
    double totalTime = 0.0;
    int totalTransfers = 0;

    for (const auto& trip : trips) {
        RouteResult res = router.findShortestPath(trip.originId, trip.destinationId);
        if (res.isReachable()) {
            routed++;
            totalTime += res.totalTimeMin;
            totalTransfers += res.transferCount;

            if (applyFeedback) {
                for (const auto& leg : res.legs) {
                    for (auto& edge : graph.adjList[leg.source]) {
                        if (edge.target == leg.target && edge.mode == leg.mode) {
                            edge.addFlow(1);
                        }
                    }
                }
            }
        } else {
            unreachable++;
        }
    }

    SimulationResult result;
    result.timeWindowLabel = timeWindowToString(window);
    result.simulatedCount = totalPassengers;
    result.routedCount = routed;
    result.unreachableCount = unreachable;
    result.avgTravelTimeMin = (routed > 0) ? (std::round((totalTime / routed) * 100.0) / 100.0) : 0.0;
    result.avgTransfersPerTrip = (routed > 0) ? (std::round(((double)totalTransfers / routed) * 100.0) / 100.0) : 0.0;

    // Gather all edges and sort by Volume / Capacity ratio
    std::vector<Edge> allEdges;
    for (const auto& pair : graph.adjList) {
        for (const auto& edge : pair.second) {
            allEdges.push_back(edge);
        }
    }

    std::sort(allEdges.begin(), allEdges.end(), [](const Edge& a, const Edge& b) {
        double rA = (a.capacity > 0) ? ((double)a.currentFlow / a.capacity) : 0.0;
        double rB = (b.capacity > 0) ? ((double)b.currentFlow / b.capacity) : 0.0;
        return rA > rB;
    });

    int count = 0;
    for (const auto& e : allEdges) {
        if (count++ >= 5) break;
        double vc = (e.capacity > 0) ? ((double)e.currentFlow / e.capacity) : 0.0;
        result.topCongestedEdges.push_back({
            e.routeId,
            e.source + " -> " + e.target,
            transportModeToString(e.mode),
            e.currentFlow,
            e.capacity,
            std::round(vc * 100.0) / 100.0,
            e.getEffectiveTravelTime(),
            e.baseTimeMin
        });
    }

    return result;
}
