#include "Profiler.h"
#include "Dijkstra.h"
#include "DemandSimulator.h"
#include <chrono>
#include <iostream>
#include <iomanip>

std::vector<BenchmarkPoint> TransitProfiler::runScalingBenchmark(const std::vector<int>& batchSizes) {
    std::vector<BenchmarkPoint> results;
    DemandSimulator simulator(const_cast<MultiModalGraph&>(graph));
    MultiModalDijkstra router(graph);

    for (int n : batchSizes) {
        auto trips = simulator.generateTrips(TimeWindow::MORNING_PEAK, n);

        auto start = std::chrono::high_resolution_clock::now();

        for (const auto& trip : trips) {
            router.findShortestPath(trip.originId, trip.destinationId);
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedSec = end - start;

        double totalSec = elapsedSec.count();
        double avgMicro = (n > 0) ? ((totalSec / n) * 1e6) : 0.0;
        double qps = (totalSec > 0.0) ? (n / totalSec) : 0.0;

        results.push_back({n, totalSec, avgMicro, qps});
    }

    return results;
}

void TransitProfiler::printSpaceComplexityAnalysis() const {
    GraphStats stats = graph.getStats();
    int v = stats.vertexCount;
    int e = stats.totalEdges;

    std::cout << "\n--- Theoretical & Practical Space Complexity ---\n";
    std::cout << " Vertices V (Stations & Hubs)     : " << v << "\n";
    std::cout << " Directed Edges E (Roads & Rails) : " << e << "\n";
    std::cout << " Adjacency List Space Complexity  : O(V + E) = O(" << v << " + " << e << ") = "
              << (v + e) << " memory records\n";
    std::cout << " Adjacency Matrix Comparison      : O(V^2) = " << (v * v) << " cells (Wasted sparse memory)\n";
    std::cout << " Memory Efficiency Ratio          : "
              << std::fixed << std::setprecision(2)
              << (100.0 * (v + e) / (v * v)) << "% of dense matrix size!\n";
    std::cout << " State Space in Dijkstra          : |V| * |Modes| = " << v << " * 2 = "
              << (v * 2) << " expanded states\n";
}
