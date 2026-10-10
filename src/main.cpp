#include "Graph.h"
#include "RoutePlanner.h"
#include "DemandSimulator.h"
#include "Profiler.h"
#include "Visualizer.h"
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

void printBanner() {
    cout << "\n"
         << "=================================================================================\n"
         << "   SMART CITY PUBLIC TRANSIT ROUTING & PROFILING SYSTEM (C++17)                 \n"
         << "   High-Performance Multi-Modal Routing Engine & Algorithmic Profiler Suite      \n"
         << "   Focus  : State-Augmented Dijkstra, Dynamic BPR Congestion & Profiler Suite   \n"
         << "=================================================================================\n";
}

void menuViewStats(const MultiModalGraph& graph) {
    GraphStats stats = graph.getStats();
    cout << "\n--- Smart City Graph Topology & Stats ---\n";
    cout << " Total Stations / Stops (Vertices V) : " << stats.vertexCount << "\n";
    cout << " Total Transit Routes (Edges E)      : " << stats.totalEdges << "\n";
    cout << "   - Bus Directed Routes             : " << stats.busEdges << "\n";
    cout << "   - Train Dedicated Tracks          : " << stats.trainEdges << "\n";
    cout << "   - Multi-Modal Interchange Hubs    : " << stats.interchangeHubs << "\n";
    cout << "\nStations Directory:\n";

    for (const auto& pair : graph.nodes) {
        const auto& node = pair.second;
        string modeTag = node.isInterchange() ? "[HUB]" : (node.nodeType == NodeType::TRAIN_STATION ? "[TRAIN]" : "[BUS]");
        cout << "  [" << node.id << "] " << left << setw(28) << node.name
             << " | Type: " << setw(8) << modeTag
             << " | Zone: " << node.zone << "\n";
    }
}

void menuPlanRoute(const RoutePlanner& planner, const MultiModalGraph& graph, const TransitVisualizer& visualizer) {
    cout << "\n--- Plan Multi-Modal Journey (State-Augmented Dijkstra) ---\n";
    cout << "Available Stations: ";
    for (const auto& pair : graph.nodes) {
        cout << pair.first << " ";
    }
    cout << "\n";

    string orig, dest;
    cout << "Enter Origin Station ID (e.g. ST01): ";
    if (!(cin >> orig)) return;
    cout << "Enter Destination Station ID (e.g. ST08): ";
    if (!(cin >> dest)) return;

    if (graph.nodes.find(orig) == graph.nodes.end() || graph.nodes.find(dest) == graph.nodes.end()) {
        cout << "[!] Error: Invalid station ID provided.\n";
        return;
    }

    cout << "\nSelect Routing Preference (Pareto Optimization):\n";
    cout << "  1. Fastest Travel Time (Standard multi-modal transfer)\n";
    cout << "  2. Minimum Transfers (Prioritize direct routes for elderly / luggage)\n";
    cout << "  3. Compare Both (Side-by-side Pareto Trade-off Analysis)\n";
    cout << "Enter preference [1-3, default 1]: ";

    string prefChoice;
    cin >> prefChoice;

    if (prefChoice == "3") {
        cout << planner.comparePreferences(orig, dest) << "\n";
        RouteResult fastResult = planner.planJourney(orig, dest, RoutingPreference::FASTEST_TIME);
        if (fastResult.isReachable()) {
            string routeSvg = "city_transit_route.svg";
            visualizer.exportSVG(routeSvg, &fastResult, false, "Optimal Journey: " + orig + " -> " + dest);
        }
        return;
    }

    RoutingPreference pref = (prefChoice == "2") ? RoutingPreference::MINIMUM_TRANSFERS : RoutingPreference::FASTEST_TIME;
    RouteResult result = planner.planJourney(orig, dest, pref);
    cout << "\n" << planner.formatItinerary(result) << "\n";

    if (result.isReachable()) {
        string routeSvg = "city_transit_route.svg";
        if (visualizer.exportSVG(routeSvg, &result, false, "Journey (" + routingPreferenceToString(pref) + "): " + orig + " -> " + dest)) {
            cout << "[+] Route Highlighted Map exported: " << routeSvg << "\n";
            cout << "    (Open '" << routeSvg << "' in your browser to inspect the route)\n";
        }
    }
}

void menuSimulateDemand(MultiModalGraph& graph, const TransitVisualizer& visualizer) {
    cout << "\n--- Variable Passenger Demand Simulation (BPR Model) ---\n";
    cout << "Select Time-of-Day Window:\n";
    cout << "1. Morning Peak    (07:00 - 09:30 AM) - Inbound commute to commercial centers\n";
    cout << "2. Midday Off-Peak (11:00 AM - 02:00 PM) - Moderate, distributed trips\n";
    cout << "3. Evening Peak    (04:30 - 07:30 PM) - Outbound commute back to suburbs\n";
    cout << "4. Night Low       (10:00 PM - 04:00 AM) - Sparse night transit\n";
    cout << "Enter option [1-4] (default 1): ";

    string opt;
    cin >> opt;

    TimeWindow window = TimeWindow::MORNING_PEAK;
    if (opt == "2") window = TimeWindow::MIDDAY_OFF_PEAK;
    else if (opt == "3") window = TimeWindow::EVENING_PEAK;
    else if (opt == "4") window = TimeWindow::NIGHT_LOW;

    cout << "Enter number of passengers to simulate (e.g. 300): ";
    int passengers = 300;
    if (!(cin >> passengers) || passengers <= 0) {
        passengers = 300;
        cin.clear();
    }

    DemandSimulator simulator(graph);
    SimulationResult res = simulator.simulateTimeWindow(window, passengers);

    cout << "\n" << string(75, '=') << "\n";
    cout << " SIMULATION REPORT: " << res.timeWindowLabel << "\n";
    cout << string(75, '=') << "\n";
    cout << " Passengers Routed   : " << res.routedCount << " / " << res.simulatedCount << "\n";
    cout << " Average Trip Time   : " << fixed << setprecision(1) << res.avgTravelTimeMin << " mins\n";
    cout << " Average Transfers   : " << setprecision(2) << res.avgTransfersPerTrip << " per trip\n";
    cout << "\nTop Bottlenecks & Congested Corridors (BPR Formula):\n";
    cout << " " << left << setw(12) << "Route ID"
         << setw(22) << "Corridor"
         << setw(8) << "Mode"
         << setw(16) << "Flow / Cap"
         << "Congestion Delay\n";
    cout << string(75, '-') << "\n";

    for (const auto& c : res.topCongestedEdges) {
        string ratioStr = to_string(c.flow) + "/" + to_string(c.capacity) +
                          " (" + to_string(static_cast<int>(c.vcRatio * 100)) + "%)";
        string delayStr = to_string(static_cast<int>(c.baseTime)) + "m -> " +
                          to_string(static_cast<int>(c.effectiveTime)) + "m";

        cout << " " << left << setw(12) << c.routeId
             << setw(22) << c.corridor
             << setw(8) << c.mode
             << setw(16) << ratioStr
             << delayStr << "\n";
    }
    cout << string(75, '=') << "\n";

    string congSvg = "city_transit_network_congested.svg";
    if (visualizer.exportSVG(congSvg, nullptr, true, "Transit Network - Congestion Heatmap (" + res.timeWindowLabel + ")")) {
        cout << "[+] Congestion-themed SVG map saved: " << congSvg << "\n";
    }
}

void menuProfiling(const MultiModalGraph& graph) {
    TransitProfiler profiler(graph);
    cout << "\n--- Algorithmic Profiling & Benchmarking Suite ---\n";
    cout << "1. Run Single Batch Benchmark (Microsecond Latency & QPS)\n";
    cout << "2. Run Scaling & Throughput Test (N = 100 to 25,000 queries) & Export CSV\n";
    cout << "3. Space Complexity Analysis (O(V+E) vs O(V^2))\n";
    cout << "Select option [1-3]: ";

    string opt;
    cin >> opt;

    if (opt == "1") {
        int n = 1000;
        cout << "Enter number of queries to profile (e.g. 1000, 10000): ";
        if (!(cin >> n) || n <= 0) n = 1000;

        cout << "\nMeasuring microsecond execution time across N = " << n << " queries...\n";
        BenchmarkPoint pt = profiler.runSingleBenchmark(n);
        TransitProfiler::printReport(pt);
    } else if (opt == "2") {
        cout << "\nExecuting multi-tier scalability benchmark suite across loads (N = 100 to 25,000)...\n";
        auto benchmarks = profiler.runScalingBenchmark({100, 500, 2000, 10000, 25000});
        TransitProfiler::printScalingTable(benchmarks);

        string csvFile = "benchmark_scaling.csv";
        if (TransitProfiler::exportToCSV(benchmarks, csvFile)) {
            cout << "[+] Benchmark scaling dataset exported to: " << csvFile << "\n";
        }
    } else {
        profiler.printSpaceComplexityAnalysis();
    }
}

void menuVisualize(const TransitVisualizer& visualizer, const RoutePlanner& planner) {
    cout << "\n--- Export Transit Network Visualizations ---\n";
    cout << "1. Export Standalone City Transit Map (city_transit_network.svg)\n";
    cout << "2. Export Transit Map with Example Journey Highlighted (ST01 -> ST08)\n";
    cout << "3. Export Graphviz DOT Topology File (transit_network.dot)\n";
    cout << "Select option [1-3]: ";

    string opt;
    cin >> opt;

    if (opt == "2") {
        RouteResult route = planner.planJourney("ST01", "ST08");
        if (visualizer.exportSVG("city_transit_route.svg", &route)) {
            cout << "[+] SUCCESS: Saved highlighted vector map to 'city_transit_route.svg'!\n";
        }
    } else if (opt == "3") {
        if (visualizer.exportDOT("transit_network.dot")) {
            cout << "[+] SUCCESS: Saved Graphviz DOT graph to 'transit_network.dot'!\n";
        }
    } else {
        if (visualizer.exportSVG("city_transit_network.svg")) {
            cout << "[+] SUCCESS: Saved full vector map to 'city_transit_network.svg'!\n";
            cout << "    (Open 'city_transit_network.svg' in any modern web browser)\n";
        }
    }
}

int main() {
    printBanner();

    MultiModalGraph graph;
    string stationsPath = "data/stations.csv";
    string routesPath = "data/routes.csv";

    if (!graph.loadFromCSV(stationsPath, routesPath)) {
        cerr << "[!] Failed to load transit data files from 'data/' directory.\n";
        return 1;
    }

    RoutePlanner planner(graph);
    TransitVisualizer visualizer(graph);

    while (true) {
        cout << "\n" << string(55, '=') << "\n";
        cout << " MAIN CLI MENU - SMART CITY TRANSIT SYSTEM (C++17)\n";
        cout << string(55, '=') << "\n";
        cout << " 1. View Transit Network Topology & Graph Stats\n";
        cout << " 2. Plan Multi-Modal Journey (Train + Bus + Transfer)\n";
        cout << " 3. Simulate Variable Demand (Peak / Off-Peak)\n";
        cout << " 4. Algorithmic Profiling & Scaling Benchmarks\n";
        cout << " 5. Export Network Visual Maps (SVG & DOT)\n";
        cout << " 6. Exit System\n";
        cout << "\nEnter choice [1-6]: ";

        string choice;
        if (!(cin >> choice)) break;

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
            cout << "\nExiting Smart City Transit System.\n\n";
            break;
        } else {
            cout << "[!] Invalid choice. Please select 1 through 6.\n";
        }
    }

    return 0;
}
