#include "Graph.h"
#include "RoutePlanner.h"
#include "DemandSimulator.h"
#include "Profiler.h"
#include "Visualizer.h"
#include <iostream>
#include <iomanip>
#include <string>

void printBanner() {
    std::cout << "\n"
              << "========================================================================\n"
              << "   SMART CITY PUBLIC TRANSIT ROUTING SYSTEM (C++17)\n"
              << "   Advanced Data Structures & Algorithms | Multi-Modal Graph Engine\n"
              << "========================================================================\n";
}

void menuViewStats(const MultiModalGraph& graph) {
    GraphStats stats = graph.getStats();
    std::cout << "\n--- Smart City Graph Topology & Stats ---\n";
    std::cout << " Total Stations / Stops (Vertices V) : " << stats.vertexCount << "\n";
    std::cout << " Total Transit Routes (Edges E)      : " << stats.totalEdges << "\n";
    std::cout << "   - Bus Directed Routes             : " << stats.busEdges << "\n";
    std::cout << "   - Train Tracks                    : " << stats.trainEdges << "\n";
    std::cout << "   - Multi-Modal Interchange Hubs    : " << stats.interchangeHubs << "\n";
    std::cout << "\nStations Directory:\n";

    for (const auto& pair : graph.nodes) {
        const auto& node = pair.second;
        std::string modeTag = node.isInterchange() ? "[HUB]" : (node.nodeType == NodeType::TRAIN_STATION ? "[TRAIN]" : "[BUS]");
        std::cout << "  [" << node.id << "] " << std::left << std::setw(28) << node.name
                  << " | Type: " << std::setw(8) << modeTag
                  << " | Zone: " << node.zone << "\n";
    }
}

void menuPlanRoute(const RoutePlanner& planner, const MultiModalGraph& graph) {
    std::cout << "\n--- Plan Multi-Modal Journey ---\n";
    std::cout << "Available Station IDs: ";
    for (const auto& pair : graph.nodes) {
        std::cout << pair.first << " ";
    }
    std::cout << "\n";

    std::string orig, dest;
    std::cout << "Enter Origin Station ID (e.g. ST01): ";
    if (!(std::cin >> orig)) return;
    std::cout << "Enter Destination Station ID (e.g. ST08): ";
    if (!(std::cin >> dest)) return;

    if (graph.nodes.find(orig) == graph.nodes.end() || graph.nodes.find(dest) == graph.nodes.end()) {
        std::cout << "Error: Invalid station ID provided.\n";
        return;
    }

    RouteResult result = planner.planJourney(orig, dest);
    std::cout << "\n" << planner.formatItinerary(result) << "\n";
}

void menuSimulateDemand(MultiModalGraph& graph) {
    std::cout << "\n--- Variable Passenger Demand Simulation ---\n";
    std::cout << "Select Time-of-Day Window:\n";
    std::cout << "1. Morning Peak  (07:00 - 09:30 AM) - Heavy suburban commute to commercial centers\n";
    std::cout << "2. Midday Off-Peak (11:00 AM - 02:00 PM) - Moderate, distributed travel\n";
    std::cout << "3. Evening Peak  (04:30 - 07:30 PM) - Heavy outbound commute back to suburbs\n";
    std::cout << "4. Night Low     (10:00 PM - 04:00 AM) - Sparse night traffic\n";
    std::cout << "Enter option [1-4] (default 1): ";

    std::string opt;
    std::cin >> opt;

    TimeWindow window = TimeWindow::MORNING_PEAK;
    if (opt == "2") window = TimeWindow::MIDDAY_OFF_PEAK;
    else if (opt == "3") window = TimeWindow::EVENING_PEAK;
    else if (opt == "4") window = TimeWindow::NIGHT_LOW;

    std::cout << "Enter number of passengers to simulate (e.g. 300): ";
    int passengers = 300;
    if (!(std::cin >> passengers) || passengers <= 0) {
        passengers = 300;
        std::cin.clear();
    }

    std::cout << "\nSimulating " << passengers << " passenger journeys during "
              << timeWindowToString(window) << "...\n";

    DemandSimulator simulator(graph);
    SimulationResult res = simulator.simulateTimeWindow(window, passengers);

    std::cout << "\n" << std::string(68, '=') << "\n";
    std::cout << " SIMULATION RESULTS: " << res.timeWindowLabel << "\n";
    std::cout << std::string(68, '=') << "\n";
    std::cout << " Passengers Routed   : " << res.routedCount << " / " << res.simulatedCount << "\n";
    std::cout << " Average Trip Time   : " << res.avgTravelTimeMin << " mins\n";
    std::cout << " Average Transfers   : " << res.avgTransfersPerTrip << " transfers per trip\n";
    std::cout << "\nTop 5 Congested Corridors / Bottlenecks:\n";
    std::cout << " " << std::left << std::setw(12) << "Route ID"
              << std::setw(20) << "Corridor"
              << std::setw(8) << "Mode"
              << std::setw(14) << "Flow/Cap"
              << std::setw(16) << "Congestion Delay" << "\n";
    std::cout << std::string(68, '-') << "\n";

    for (const auto& c : res.topCongestedEdges) {
        std::string ratioStr = std::to_string(c.flow) + "/" + std::to_string(c.capacity) +
                               " (" + std::to_string(static_cast<int>(c.vcRatio * 100)) + "%)";
        std::string delayStr = std::to_string(static_cast<int>(c.baseTime)) + "m -> " +
                               std::to_string(static_cast<int>(c.effectiveTime)) + "m";

        std::cout << " " << std::left << std::setw(12) << c.routeId
                  << std::setw(20) << c.corridor
                  << std::setw(8) << c.mode
                  << std::setw(14) << ratioStr
                  << std::setw(16) << delayStr << "\n";
    }
    std::cout << std::string(68, '=') << "\n";
}

void menuProfiling(MultiModalGraph& graph) {
    TransitProfiler profiler(graph);
    std::cout << "\n--- Algorithmic Profiling & Benchmarking (C++ High-Resolution Clock) ---\n";
    std::cout << "1. Run Scaling & Throughput Test (N = 100, 500, 2000, 10000, 25000 queries)\n";
    std::cout << "2. Check Theoretical vs Practical Space Complexity\n";
    std::cout << "Select profiling option [1-2]: ";

    std::string opt;
    std::cin >> opt;

    if (opt == "1") {
        std::cout << "\nMeasuring native C++ execution time across query scaling loads...\n";
        auto benchmarks = profiler.runScalingBenchmark({100, 500, 2000, 10000, 25000});

        std::cout << "\n" << std::string(68, '=') << "\n";
        std::cout << " " << std::left << std::setw(14) << "Queries (N)"
                  << std::setw(16) << "Total Time (s)"
                  << std::setw(18) << "Avg/Query (us)"
                  << std::setw(18) << "Throughput (QPS)" << "\n";
        std::cout << std::string(68, '=') << "\n";

        for (const auto& b : benchmarks) {
            std::cout << " " << std::left << std::setw(14) << b.queryCount
                      << std::fixed << std::setprecision(5) << std::setw(16) << b.totalTimeSec
                      << std::setprecision(2) << std::setw(18) << b.avgLatencyMicrosec
                      << std::setprecision(0) << std::setw(18) << b.throughputQPS << "\n";
        }
        std::cout << std::string(68, '=') << "\n";
    } else {
        profiler.printSpaceComplexityAnalysis();
    }
}

void menuVisualize(const MultiModalGraph& graph, const RoutePlanner& planner) {
    TransitVisualizer visualizer(graph);
    std::cout << "\n--- Export Transit Network Map ---\n";
    std::cout << "1. Export Full City Transit Map (SVG Vector Graphic)\n";
    std::cout << "2. Export Transit Map with Example Multi-Modal Route Highlighted\n";
    std::cout << "3. Export Graphviz DOT File\n";
    std::cout << "Select option [1-3]: ";

    std::string opt;
    std::cin >> opt;

    if (opt == "2") {
        RouteResult route = planner.planJourney("ST01", "ST08");
        if (visualizer.exportSVG("city_transit_route.svg", &route)) {
            std::cout << "SUCCESS: Saved highlighted vector map to 'city_transit_route.svg'!\n";
            std::cout << "You can double click or open 'city_transit_route.svg' in any web browser.\n";
        }
    } else if (opt == "3") {
        if (visualizer.exportDOT("city_transit_network.dot")) {
            std::cout << "SUCCESS: Saved Graphviz DOT graph to 'city_transit_network.dot'!\n";
        }
    } else {
        if (visualizer.exportSVG("city_transit_network.svg")) {
            std::cout << "SUCCESS: Saved full vector map to 'city_transit_network.svg'!\n";
            std::cout << "You can double click or open 'city_transit_network.svg' in any web browser.\n";
        }
    }
}

int main() {
    printBanner();

    MultiModalGraph graph;
    std::string stationsPath = "data/stations.csv";
    std::string routesPath = "data/routes.csv";

    if (!graph.loadFromCSV(stationsPath, routesPath)) {
        std::cerr << "Failed to load transit data files.\n";
        return 1;
    }

    RoutePlanner planner(graph);

    while (true) {
        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << " MAIN MENU - SMART CITY TRANSIT SYSTEM (C++)\n";
        std::cout << std::string(50, '=') << "\n";
        std::cout << " 1. View Transit Network Topology & Graph Stats\n";
        std::cout << " 2. Plan Multi-Modal Journey (Train + Bus + Transfer)\n";
        std::cout << " 3. Simulate Variable Demand (Peak / Off-Peak)\n";
        std::cout << " 4. Algorithmic Profiling & Scaling Benchmarks\n";
        std::cout << " 5. Export Network Visual Map (SVG & DOT)\n";
        std::cout << " 6. Exit\n";
        std::cout << "\nEnter choice [1-6]: ";

        std::string choice;
        if (!(std::cin >> choice)) break;

        if (choice == "1") {
            menuViewStats(graph);
        } else if (choice == "2") {
            menuPlanRoute(planner, graph);
        } else if (choice == "3") {
            menuSimulateDemand(graph);
        } else if (choice == "4") {
            menuProfiling(graph);
        } else if (choice == "5") {
            menuVisualize(graph, planner);
        } else if (choice == "6") {
            std::cout << "\nExiting Smart City Transit System. Best of luck with the UCSC DSA Viva!\n\n";
            break;
        } else {
            std::cout << "Invalid choice. Please select 1 through 6.\n";
        }
    }

    return 0;
}
