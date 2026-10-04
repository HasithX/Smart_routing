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
              << "=================================================================================\n"
              << "   SMART CITY PUBLIC TRANSIT ROUTING & PROFILING SYSTEM (C++17)                 \n"
              << "   Course : UCSC IS 2202 / IS 2110 (Advanced Data Structures and Algorithms)     \n"
              << "   Focus  : State-Augmented Dijkstra, Dynamic BPR Congestion & Profiler Suite   \n"
              << "=================================================================================\n";
}

void menuViewStats(const MultiModalGraph& graph) {
    GraphStats stats = graph.getStats();
    std::cout << "\n--- Smart City Graph Topology & Stats ---\n";
    std::cout << " Total Stations / Stops (Vertices V) : " << stats.vertexCount << "\n";
    std::cout << " Total Transit Routes (Edges E)      : " << stats.totalEdges << "\n";
    std::cout << "   - Bus Directed Routes             : " << stats.busEdges << "\n";
    std::cout << "   - Train Dedicated Tracks          : " << stats.trainEdges << "\n";
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

void menuPlanRoute(const RoutePlanner& planner, const MultiModalGraph& graph, const TransitVisualizer& visualizer) {
    std::cout << "\n--- Plan Multi-Modal Journey (State-Augmented Dijkstra) ---\n";
    std::cout << "Available Stations: ";
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
        std::cout << "[!] Error: Invalid station ID provided.\n";
        return;
    }

    std::cout << "\nSelect Routing Preference (Pareto Optimization):\n";
    std::cout << "  1. Fastest Travel Time (Standard multi-modal transfer)\n";
    std::cout << "  2. Minimum Transfers (Prioritize direct routes for elderly / luggage)\n";
    std::cout << "  3. Compare Both (Side-by-side Pareto Trade-off Analysis)\n";
    std::cout << "Enter preference [1-3, default 1]: ";

    std::string prefChoice;
    std::cin >> prefChoice;

    if (prefChoice == "3") {
        std::cout << planner.comparePreferences(orig, dest) << "\n";
        RouteResult fastResult = planner.planJourney(orig, dest, RoutingPreference::FASTEST_TIME);
        if (fastResult.isReachable()) {
            std::string routeSvg = "city_transit_route.svg";
            visualizer.exportSVG(routeSvg, &fastResult, false, "Optimal Journey: " + orig + " -> " + dest);
        }
        return;
    }

    RoutingPreference pref = (prefChoice == "2") ? RoutingPreference::MINIMUM_TRANSFERS : RoutingPreference::FASTEST_TIME;
    RouteResult result = planner.planJourney(orig, dest, pref);
    std::cout << "\n" << planner.formatItinerary(result) << "\n";

    if (result.isReachable()) {
        std::string routeSvg = "city_transit_route.svg";
        if (visualizer.exportSVG(routeSvg, &result, false, "Journey (" + routingPreferenceToString(pref) + "): " + orig + " -> " + dest)) {
            std::cout << "[+] Route Highlighted Map exported: " << routeSvg << "\n";
            std::cout << "    (Open '" << routeSvg << "' in your browser to inspect the route)\n";
        }
    }
}

void menuSimulateDemand(MultiModalGraph& graph, const TransitVisualizer& visualizer) {
    std::cout << "\n--- Variable Passenger Demand Simulation (BPR Model) ---\n";
    std::cout << "Select Time-of-Day Window:\n";
    std::cout << "1. Morning Peak    (07:00 - 09:30 AM) - Inbound commute to commercial centers\n";
    std::cout << "2. Midday Off-Peak (11:00 AM - 02:00 PM) - Moderate, distributed trips\n";
    std::cout << "3. Evening Peak    (04:30 - 07:30 PM) - Outbound commute back to suburbs\n";
    std::cout << "4. Night Low       (10:00 PM - 04:00 AM) - Sparse night transit\n";
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

    DemandSimulator simulator(graph);
    SimulationResult res = simulator.simulateTimeWindow(window, passengers);

    std::cout << "\n" << std::string(75, '=') << "\n";
    std::cout << " SIMULATION REPORT: " << res.timeWindowLabel << "\n";
    std::cout << std::string(75, '=') << "\n";
    std::cout << " Passengers Routed   : " << res.routedCount << " / " << res.simulatedCount << "\n";
    std::cout << " Average Trip Time   : " << std::fixed << std::setprecision(1) << res.avgTravelTimeMin << " mins\n";
    std::cout << " Average Transfers   : " << std::setprecision(2) << res.avgTransfersPerTrip << " per trip\n";
    std::cout << "\nTop Bottlenecks & Congested Corridors (BPR Formula):\n";
    std::cout << " " << std::left << std::setw(12) << "Route ID"
              << std::setw(22) << "Corridor"
              << std::setw(8) << "Mode"
              << std::setw(16) << "Flow / Cap"
              << "Congestion Delay\n";
    std::cout << std::string(75, '-') << "\n";

    for (const auto& c : res.topCongestedEdges) {
        std::string ratioStr = std::to_string(c.flow) + "/" + std::to_string(c.capacity) +
                               " (" + std::to_string(static_cast<int>(c.vcRatio * 100)) + "%)";
        std::string delayStr = std::to_string(static_cast<int>(c.baseTime)) + "m -> " +
                               std::to_string(static_cast<int>(c.effectiveTime)) + "m";

        std::cout << " " << std::left << std::setw(12) << c.routeId
                  << std::setw(22) << c.corridor
                  << std::setw(8) << c.mode
                  << std::setw(16) << ratioStr
                  << delayStr << "\n";
    }
    std::cout << std::string(75, '=') << "\n";

    std::string congSvg = "city_transit_network_congested.svg";
    if (visualizer.exportSVG(congSvg, nullptr, true, "Colombo Transit Network - Congestion Heatmap (" + res.timeWindowLabel + ")")) {
        std::cout << "[+] Congestion-themed SVG map saved: " << congSvg << "\n";
    }
}

void menuProfiling(const MultiModalGraph& graph) {
    TransitProfiler profiler(graph);
    std::cout << "\n--- Algorithmic Profiling & Benchmarking (Member 4 Suite) ---\n";
    std::cout << "1. Run Single Batch Benchmark (Microsecond Latency & QPS)\n";
    std::cout << "2. Run Scaling & Throughput Test (N = 100 to 25,000 queries) & Export CSV\n";
    std::cout << "3. Space Complexity & UCSC Viva DSA Analysis (O(V+E) vs O(V^2))\n";
    std::cout << "Select option [1-3]: ";

    std::string opt;
    std::cin >> opt;

    if (opt == "1") {
        int n = 1000;
        std::cout << "Enter number of queries to profile (e.g. 1000, 10000): ";
        if (!(std::cin >> n) || n <= 0) n = 1000;

        std::cout << "\nMeasuring microsecond execution time across N = " << n << " queries...\n";
        BenchmarkPoint pt = profiler.runSingleBenchmark(n);
        TransitProfiler::printReport(pt);
    } else if (opt == "2") {
        std::cout << "\nExecuting multi-tier scalability benchmark suite across loads (N = 100 to 25,000)...\n";
        auto benchmarks = profiler.runScalingBenchmark({100, 500, 2000, 10000, 25000});
        TransitProfiler::printScalingTable(benchmarks);

        std::string csvFile = "benchmark_scaling.csv";
        if (TransitProfiler::exportToCSV(benchmarks, csvFile)) {
            std::cout << "[+] Benchmark scaling dataset exported to: " << csvFile << "\n";
        }
    } else {
        profiler.printSpaceComplexityAnalysis();
    }
}

void menuVisualize(const TransitVisualizer& visualizer, const RoutePlanner& planner) {
    std::cout << "\n--- Export Transit Network Visualizations ---\n";
    std::cout << "1. Export Standalone City Transit Map (city_transit_network.svg)\n";
    std::cout << "2. Export Transit Map with Example Journey Highlighted (ST01 -> ST08)\n";
    std::cout << "3. Export Graphviz DOT Topology File (transit_network.dot)\n";
    std::cout << "Select option [1-3]: ";

    std::string opt;
    std::cin >> opt;

    if (opt == "2") {
        RouteResult route = planner.planJourney("ST01", "ST08");
        if (visualizer.exportSVG("city_transit_route.svg", &route)) {
            std::cout << "[+] SUCCESS: Saved highlighted vector map to 'city_transit_route.svg'!\n";
        }
    } else if (opt == "3") {
        if (visualizer.exportDOT("transit_network.dot")) {
            std::cout << "[+] SUCCESS: Saved Graphviz DOT graph to 'transit_network.dot'!\n";
        }
    } else {
        if (visualizer.exportSVG("city_transit_network.svg")) {
            std::cout << "[+] SUCCESS: Saved full vector map to 'city_transit_network.svg'!\n";
            std::cout << "    (Open 'city_transit_network.svg' in any modern web browser)\n";
        }
    }
}

int main() {
    printBanner();

    MultiModalGraph graph;
    std::string stationsPath = "data/stations.csv";
    std::string routesPath = "data/routes.csv";

    if (!graph.loadFromCSV(stationsPath, routesPath)) {
        std::cerr << "[!] Failed to load transit data files from 'data/' directory.\n";
        return 1;
    }

    RoutePlanner planner(graph);
    TransitVisualizer visualizer(graph);

    while (true) {
        std::cout << "\n" << std::string(55, '=') << "\n";
        std::cout << " MAIN CLI MENU - SMART CITY TRANSIT SYSTEM (C++17)\n";
        std::cout << std::string(55, '=') << "\n";
        std::cout << " 1. View Transit Network Topology & Graph Stats\n";
        std::cout << " 2. Plan Multi-Modal Journey (Train + Bus + Transfer)\n";
        std::cout << " 3. Simulate Variable Demand (Peak / Off-Peak)\n";
        std::cout << " 4. Algorithmic Profiling & Scaling Benchmarks (Member 4)\n";
        std::cout << " 5. Export Network Visual Maps (SVG & DOT) (Member 4)\n";
        std::cout << " 6. Exit System\n";
        std::cout << "\nEnter choice [1-6]: ";

        std::string choice;
        if (!(std::cin >> choice)) break;

        if (choice == "1") {
            menuViewStats(graph);
        } else if (choice == "2") {
            menuPlanRoute(planner, graph, visualizer);
        } else if (choice == "3") {
            menuSimulateDemand(graph, visualizer);
        } else if (choice == "4") {
            menuProfiling(graph);
        } else if (choice == "5") {
            menuVisualize(visualizer, planner);
        } else if (choice == "6") {
            std::cout << "\nExiting Smart City Transit System. Best of luck with UCSC IS 2202/2110!\n\n";
            break;
        } else {
            std::cout << "[!] Invalid choice. Please select 1 through 6.\n";
        }
    }

    return 0;
}
