#include "DemandSimulator.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

double volumeToCapacity(const Edge& edge)
{
    if (edge.capacity <= 0) {
        return 0.0;
    }

    return static_cast<double>(edge.currentFlow) /
           static_cast<double>(edge.capacity);
}

bool isSameLeg(const RouteLeg& a, const RouteLeg& b)
{
    return a.source == b.source &&
           a.target == b.target &&
           a.mode == b.mode;
}

bool sameRoute(const RouteResult& a, const RouteResult& b)
{
    if (!a.isReachable() || !b.isReachable()) {
        return false;
    }

    if (a.legs.size() != b.legs.size()) {
        return false;
    }

    for (std::size_t i = 0; i < a.legs.size(); ++i) {
        if (!isSameLeg(a.legs[i], b.legs[i])) {
            return false;
        }
    }

    return true;
}

void addPassengerFlow(MultiModalGraph& graph, const RouteResult& route)
{
    for (const auto& leg : route.legs) {
        auto it = graph.adjList.find(leg.source);

        if (it == graph.adjList.end()) {
            continue;
        }

        for (auto& edge : it->second) {
            if (edge.target == leg.target &&
                edge.mode == leg.mode) {
                edge.addFlow(1);
                break;
            }
        }
    }
}

} // namespace

// ============================================================
// TIME WINDOW TO STRING
// ============================================================

std::string timeWindowToString(TimeWindow tw)
{
    switch (tw) {
        case TimeWindow::MORNING_PEAK:
            return "07:00 - 09:30 AM (Morning Peak Commute)";

        case TimeWindow::MIDDAY_OFF_PEAK:
            return "11:00 AM - 02:00 PM (Midday Off-Peak)";

        case TimeWindow::EVENING_PEAK:
            return "04:30 - 07:30 PM (Evening Peak Commute)";

        case TimeWindow::NIGHT_LOW:
            return "10:00 PM - 04:00 AM (Night Low Demand)";
    }

    return "Standard Window";
}

// ============================================================
// GENERATE PASSENGER TRIPS
// ============================================================

std::vector<PassengerTrip> DemandSimulator::generateTrips(
    TimeWindow window,
    int totalPassengers,
    unsigned int seed)
{
    std::vector<PassengerTrip> trips;

    if (totalPassengers <= 0) {
        return trips;
    }

    std::mt19937 rng(seed);

    std::vector<std::string> allNodeIds;
    std::vector<std::string> residentialNodes;
    std::vector<std::string> commercialNodes;

    // Classify stations by zone.
    for (const auto& pair : graph.nodes) {
        allNodeIds.push_back(pair.first);

        std::string zone = pair.second.zone;
        std::transform(
            zone.begin(),
            zone.end(),
            zone.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });

        if (zone == "residential") {
            residentialNodes.push_back(pair.first);
        } else if (
            zone == "commercial" ||
            zone == "industrial" ||
            zone == "educational" ||
            zone == "administrative") {
            commercialNodes.push_back(pair.first);
        }
    }

    // Fallback so the simulator still works with a different dataset.
    if (residentialNodes.empty()) {
        residentialNodes = allNodeIds;
    }

    if (commercialNodes.empty()) {
        commercialNodes = allNodeIds;
    }

    if (allNodeIds.size() < 2) {
        return trips;
    }

    std::uniform_real_distribution<double> probability(0.0, 1.0);
    std::uniform_int_distribution<int> minute(0, 59);

    trips.reserve(static_cast<std::size_t>(totalPassengers));

    for (int i = 0; i < totalPassengers; ++i) {
        char buffer[32];

        std::snprintf(
            buffer,
            sizeof(buffer),
            "PX_%04d",
            i + 1
        );

        std::string origin;
        std::string destination;
        std::string departureTime;

        // Morning: 80% residential -> work/commercial.
        if (window == TimeWindow::MORNING_PEAK) {
            if (probability(rng) < 0.80) {
                origin = residentialNodes[
                    rng() % residentialNodes.size()
                ];

                destination = commercialNodes[
                    rng() % commercialNodes.size()
                ];
            } else {
                origin = allNodeIds[
                    rng() % allNodeIds.size()
                ];

                destination = allNodeIds[
                    rng() % allNodeIds.size()
                ];
            }

            std::snprintf(
                buffer,
                sizeof(buffer),
                "07:%02d AM",
                minute(rng)
            );
            departureTime = buffer;
        }

        // Evening: 80% commercial/work -> residential.
        else if (window == TimeWindow::EVENING_PEAK) {
            if (probability(rng) < 0.80) {
                origin = commercialNodes[
                    rng() % commercialNodes.size()
                ];

                destination = residentialNodes[
                    rng() % residentialNodes.size()
                ];
            } else {
                origin = allNodeIds[
                    rng() % allNodeIds.size()
                ];

                destination = allNodeIds[
                    rng() % allNodeIds.size()
                ];
            }

            std::snprintf(
                buffer,
                sizeof(buffer),
                "05:%02d PM",
                minute(rng)
            );
            departureTime = buffer;
        }

        // Midday: distributed/random demand.
        else if (window == TimeWindow::MIDDAY_OFF_PEAK) {
            origin = allNodeIds[
                rng() % allNodeIds.size()
            ];

            destination = allNodeIds[
                rng() % allNodeIds.size()
            ];

            std::snprintf(
                buffer,
                sizeof(buffer),
                "01:%02d PM",
                minute(rng)
            );
            departureTime = buffer;
        }

        // Night: sparse/random demand.
        else {
            origin = allNodeIds[
                rng() % allNodeIds.size()
            ];

            destination = allNodeIds[
                rng() % allNodeIds.size()
            ];

            std::snprintf(
                buffer,
                sizeof(buffer),
                "11:%02d PM",
                minute(rng)
            );
            departureTime = buffer;
        }

        // Never create a trip from a station to itself.
        while (origin == destination) {
            destination = allNodeIds[
                rng() % allNodeIds.size()
            ];
        }

        std::snprintf(
            buffer,
            sizeof(buffer),
            "PX_%04d",
            i + 1
        );

        trips.push_back({
            std::string(buffer),
            origin,
            destination,
            departureTime,
            window
        });
    }

    return trips;
}

// ============================================================
// SIMULATE ONE TIME WINDOW
// ============================================================

SimulationResult DemandSimulator::simulateTimeWindow(
    TimeWindow window,
    int totalPassengers,
    bool applyFeedback)
{
    graph.resetAllCongestion();

    auto trips = generateTrips(window, totalPassengers);

    MultiModalDijkstra router(graph);

    int routed = 0;
    int unreachable = 0;
    double totalTravelTime = 0.0;
    int totalTransfers = 0;

    for (const auto& trip : trips) {
        RouteResult route = router.findShortestPath(
            trip.originId,
            trip.destinationId
        );

        if (!route.isReachable()) {
            ++unreachable;
            continue;
        }

        ++routed;
        totalTravelTime += route.totalTimeMin;
        totalTransfers += route.transferCount;

        if (applyFeedback) {
            addPassengerFlow(graph, route);
        }
    }

    SimulationResult result;

    result.timeWindowLabel = timeWindowToString(window);
    result.simulatedCount = totalPassengers;
    result.routedCount = routed;
    result.unreachableCount = unreachable;

    result.avgTravelTimeMin =
        routed > 0
        ? std::round((totalTravelTime / routed) * 100.0) / 100.0
        : 0.0;

    result.avgTransfersPerTrip =
        routed > 0
        ? std::round(
              (static_cast<double>(totalTransfers) / routed) * 100.0
          ) / 100.0
        : 0.0;

    // Rank the five most congested corridors by V/C ratio.
    std::vector<Edge> allEdges;

    for (const auto& pair : graph.adjList) {
        for (const auto& edge : pair.second) {
            allEdges.push_back(edge);
        }
    }

    std::sort(
        allEdges.begin(),
        allEdges.end(),
        [](const Edge& a, const Edge& b) {
            return volumeToCapacity(a) > volumeToCapacity(b);
        }
    );

    const int maxResults = 5;

    for (int i = 0;
         i < static_cast<int>(allEdges.size()) && i < maxResults;
         ++i) {

        const Edge& edge = allEdges[i];
        double ratio = volumeToCapacity(edge);

        result.topCongestedEdges.push_back({
            edge.routeId,
            edge.source + " -> " + edge.target,
            transportModeToString(edge.mode),
            edge.currentFlow,
            edge.capacity,
            std::round(ratio * 100.0) / 100.0,
            edge.getEffectiveTravelTime(),
            edge.baseTimeMin
        });
    }

    return result;
}

// ============================================================
// 24-HOUR DYNAMIC DEMAND
// ============================================================

std::vector<int> DemandSimulator::generate24HourDemand(int baseDemand)
{
    std::vector<int> demand(24, 0);

    if (baseDemand <= 0) {
        return demand;
    }

    // Relative demand multiplier for each hour.
    const double multipliers[24] = {
        0.15, 0.10, 0.08, 0.08, 0.10, 0.15,
        0.30, 0.70, 1.00, 0.75, 0.55, 0.50,
        0.45, 0.50, 0.55, 0.65, 0.80, 0.95,
        1.00, 0.85, 0.65, 0.45, 0.30, 0.20
    };

    for (int hour = 0; hour < 24; ++hour) {
        demand[hour] = static_cast<int>(
            std::round(baseDemand * multipliers[hour])
        );
    }

    return demand;
}

// ============================================================
// EMERGENCY DISRUPTION SIMULATOR
// ============================================================

void DemandSimulator::simulateDisruption(const std::string& routeId)
{
    std::cout << "\n"
              << "============================================================\n"
              << "       EMERGENCY TRANSPORT DISRUPTION SIMULATOR\n"
              << "============================================================\n"
              << "Disrupted Route: " << routeId << "\n";

    // Because the graph creates a reverse edge with "_REV", block
    // both directions of the selected physical route.
    const std::string reverseRouteId = routeId + "_REV";

    bool routeFound = false;
    std::string routeSource;
    std::string routeTarget;
    TransportMode disruptedMode = TransportMode::TRAIN;
    double baseTravelTime = 0.0;

    for (const auto& pair : graph.adjList) {
        for (const auto& edge : pair.second) {
            if (edge.routeId == routeId ||
                edge.routeId == reverseRouteId) {

                routeFound = true;
                routeSource = edge.source;
                routeTarget = edge.target;
                disruptedMode = edge.mode;
                baseTravelTime = edge.baseTimeMin;
                break;
            }
        }

        if (routeFound) {
            break;
        }
    }

    if (!routeFound) {
        std::cout << "\nERROR: Route '" << routeId
                  << "' was not found.\n";
        return;
    }

    std::cout << "Route: " << routeSource
              << " -> " << routeTarget << "\n"
              << "Mode: " << transportModeToString(disruptedMode) << "\n"
              << "Base Travel Time: " << baseTravelTime << " min\n";

    // Use the same 300 passenger trips in both scenarios so the
    // comparison is fair.
    const int passengerCount = 300;

    auto trips = generateTrips(
        TimeWindow::MORNING_PEAK,
        passengerCount,
        42
    );

    MultiModalDijkstra router(graph);

    // ---------------- NORMAL SCENARIO ----------------

    graph.resetAllCongestion();

    std::vector<RouteResult> normalRoutes;
    normalRoutes.reserve(trips.size());

    int normalRouted = 0;
    double normalTotalTime = 0.0;

    for (const auto& trip : trips) {
        RouteResult route = router.findShortestPath(
            trip.originId,
            trip.destinationId
        );

        normalRoutes.push_back(route);

        if (route.isReachable()) {
            ++normalRouted;
            normalTotalTime += route.totalTimeMin;
        }
    }

    const double normalAverageTime =
        normalRouted > 0
        ? normalTotalTime / normalRouted
        : 0.0;

    // ---------------- BLOCK THE FAILED ROUTE ----------------

    std::vector<std::pair<std::string, Edge>> removedEdges;

    for (auto& pair : graph.adjList) {
        auto& edges = pair.second;

        auto it = edges.begin();

        while (it != edges.end()) {
            if (it->routeId == routeId ||
                it->routeId == reverseRouteId) {

                removedEdges.emplace_back(pair.first, *it);
                it = edges.erase(it);
            } else {
                ++it;
            }
        }
    }

    std::cout << "\nSTATUS: Route " << routeId
              << " is BLOCKED in both directions.\n";

    // ---------------- DISRUPTED SCENARIO ----------------

    graph.resetAllCongestion();

    int disruptedRouted = 0;
    int disruptedUnreachable = 0;
    int divertedPassengers = 0;
    int passengersUsingBus = 0;
    double disruptedTotalTime = 0.0;

    for (std::size_t i = 0; i < trips.size(); ++i) {
        const auto& trip = trips[i];

        RouteResult route = router.findShortestPath(
            trip.originId,
            trip.destinationId
        );

        if (!route.isReachable()) {
            ++disruptedUnreachable;
            continue;
        }

        ++disruptedRouted;
        disruptedTotalTime += route.totalTimeMin;

        // A passenger is counted as diverted only when the route
        // selected after the failure differs from the normal route.
        if (i < normalRoutes.size() &&
            normalRoutes[i].isReachable() &&
            !sameRoute(normalRoutes[i], route)) {
            ++divertedPassengers;
        }

        bool usesBus = false;

        for (const auto& leg : route.legs) {
            if (leg.mode == TransportMode::BUS) {
                usesBus = true;
            }
        }

        if (usesBus) {
            ++passengersUsingBus;
        }

        addPassengerFlow(graph, route);
    }

    const double disruptedAverageTime =
        disruptedRouted > 0
        ? disruptedTotalTime / disruptedRouted
        : 0.0;

    const double additionalDelay =
        disruptedAverageTime - normalAverageTime;

    // ---------------- DISPLAY RESULTS ----------------

    std::cout << "\n---------------- DISRUPTION RESULTS ----------------\n"
              << "Passengers Simulated      : " << passengerCount << "\n"
              << "Normally Routed           : " << normalRouted << "\n"
              << "Routed After Disruption   : " << disruptedRouted << "\n"
              << "Unreachable After Failure : " << disruptedUnreachable << "\n"
              << "Passengers Diverted       : " << divertedPassengers << "\n"
              << "Passengers Using Bus      : " << passengersUsingBus << "\n";

    std::cout << std::fixed << std::setprecision(2)
              << "Normal Average Time       : " << normalAverageTime << " min\n"
              << "Disrupted Average Time    : " << disruptedAverageTime << " min\n"
              << "Additional Average Delay  : " << additionalDelay << " min\n";

    // ---------------- ALTERNATIVE BUS CONGESTION ----------------

    std::vector<Edge> busEdges;

    for (const auto& pair : graph.adjList) {
        for (const auto& edge : pair.second) {
            if (edge.mode == TransportMode::BUS &&
                edge.currentFlow > 0) {
                busEdges.push_back(edge);
            }
        }
    }

    std::sort(
        busEdges.begin(),
        busEdges.end(),
        [](const Edge& a, const Edge& b) {
            return volumeToCapacity(a) > volumeToCapacity(b);
        }
    );

    std::cout << "\n--------- MOST CONGESTED ALTERNATIVE BUS ROUTES ---------\n";

    int displayed = 0;

    for (const auto& edge : busEdges) {
        if (displayed >= 5) {
            break;
        }

        double ratio = volumeToCapacity(edge);

        std::cout << "\n"
                  << edge.routeId << " | "
                  << edge.source << " -> " << edge.target << "\n"
                  << "  Flow / Capacity : "
                  << edge.currentFlow << " / " << edge.capacity
                  << " (" << static_cast<int>(ratio * 100.0) << "%)\n"
                  << "  Travel Time     : "
                  << edge.baseTimeMin << " -> "
                  << edge.getEffectiveTravelTime() << " min\n";

        ++displayed;
    }

    if (displayed == 0) {
        std::cout << "No alternative bus congestion detected.\n";
    }

    // ---------------- RESTORE GRAPH ----------------

    for (const auto& item : removedEdges) {
        graph.adjList[item.first].push_back(item.second);
    }

    // Always leave the graph clean after the simulation.
    graph.resetAllCongestion();

    std::cout << "\nSTATUS: Route " << routeId
              << " restored.\n"
              << "============================================================\n";
}
