#include "Profiler.h"
#include "Dijkstra.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <fstream>

BenchmarkPoint TransitProfiler::runSingleBenchmark(int queryCount, unsigned int seed) const {
    BenchmarkPoint result;
    result.queryCount = queryCount;

    if (graph.nodes.size() < 2 || queryCount <= 0) {
        return result;
    }

    std::vector<std::string> nodeIds;
    nodeIds.reserve(graph.nodes.size());
    for (const auto& pair : graph.nodes) {
        nodeIds.push_back(pair.first);
    }

    std::mt19937 rng(seed);
    std::uniform_int_distribution<size_t> dist(0, nodeIds.size() - 1);

    // Pre-generate query pairs so trip generation time is excluded from routing measurements
    std::vector<std::pair<std::string, std::string>> queries;
    queries.reserve(queryCount);
    for (int i = 0; i < queryCount; ++i) {
        std::string u = nodeIds[dist(rng)];
        std::string v = nodeIds[dist(rng)];
        while (v == u) {
            v = nodeIds[dist(rng)];
        }
        queries.emplace_back(u, v);
    }

    MultiModalDijkstra router(graph);

    // Warm-up phase: 50 iterations to avoid cold-cache and CPU frequency ramp-up bias
    int warmupCount = std::min(50, queryCount);
    for (int i = 0; i < warmupCount; ++i) {
        volatile auto warm = router.findShortestPath(queries[i].first, queries[i].second);
        (void)warm;
    }

    std::vector<double> latenciesUs;
    latenciesUs.reserve(queryCount);

    auto startOverall = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < queryCount; ++i) {
        auto qStart = std::chrono::high_resolution_clock::now();
        RouteResult res = router.findShortestPath(queries[i].first, queries[i].second);
        auto qEnd = std::chrono::high_resolution_clock::now();

        double elapsedUs = std::chrono::duration<double, std::micro>(qEnd - qStart).count();
        latenciesUs.push_back(elapsedUs);

        if (res.isReachable()) {
            result.successfulRoutes++;
        } else {
            result.failedRoutes++;
        }
    }

    auto endOverall = std::chrono::high_resolution_clock::now();
    result.totalTimeSec = std::chrono::duration<double>(endOverall - startOverall).count();

    if (!latenciesUs.empty()) {
        std::sort(latenciesUs.begin(), latenciesUs.end());

        double sum = std::accumulate(latenciesUs.begin(), latenciesUs.end(), 0.0);
        result.avgLatencyMicrosec = sum / latenciesUs.size();
        result.minLatencyMicrosec = latenciesUs.front();
        result.maxLatencyMicrosec = latenciesUs.back();
        result.medianLatencyMicrosec = latenciesUs[latenciesUs.size() / 2];

        size_t idx95 = static_cast<size_t>(latenciesUs.size() * 0.95);
        size_t idx99 = static_cast<size_t>(latenciesUs.size() * 0.99);
        if (idx95 >= latenciesUs.size()) idx95 = latenciesUs.size() - 1;
        if (idx99 >= latenciesUs.size()) idx99 = latenciesUs.size() - 1;

        result.p95LatencyMicrosec = latenciesUs[idx95];
        result.p99LatencyMicrosec = latenciesUs[idx99];

        result.throughputQPS = (result.totalTimeSec > 0.0) ? (queryCount / result.totalTimeSec) : 0.0;
    }

    return result;
}

std::vector<BenchmarkPoint> TransitProfiler::runScalingBenchmark(const std::vector<int>& batchSizes) {
    std::vector<BenchmarkPoint> results;
    results.reserve(batchSizes.size());

    for (int n : batchSizes) {
        results.push_back(runSingleBenchmark(n, 42));
    }

    return results;
}

void TransitProfiler::printSpaceComplexityAnalysis() const {
    GraphStats stats = graph.getStats();
    int v = stats.vertexCount;
    int e = stats.totalEdges;
    int modes = 2; // Bus and Train

    // Theoretical calculations
    int denseMatrixCells = v * v;
    size_t estAdjListBytes = (v * sizeof(std::vector<Edge>)) + (e * sizeof(Edge)) + (v * sizeof(Location));
    size_t estMatrixBytes = static_cast<size_t>(denseMatrixCells) * sizeof(double);

    std::cout << "\n===================================================================================\n";
    std::cout << "          DATA STRUCTURE SPACE COMPLEXITY PROFILING (UCSC IS 2202/2110)            \n";
    std::cout << "===================================================================================\n";
    std::cout << "  Vertices V (Stations & Interchange Hubs) : " << v << "\n";
    std::cout << "  Directed Edges E (Roads & Rail Links)    : " << e << "\n";
    std::cout << "  Active Transit Modes |M|                 : " << modes << " (Bus, Train)\n";
    std::cout << "  Graph Density (|E| / (|V|*(|V|-1)))      : "
              << std::fixed << std::setprecision(4)
              << ((v > 1) ? (static_cast<double>(e) / (v * (v - 1))) : 0.0) << " (Sparse Transit Topology)\n";
    std::cout << "-----------------------------------------------------------------------------------\n";
    std::cout << "  Adjacency List Space Complexity          : O(V + E)\n";
    std::cout << "    - Theoretical Memory Records           : " << (v + e) << " records\n";
    std::cout << "    - Estimated RAM Allocation             : ~" << (estAdjListBytes / 1024.0) << " KB\n";
    std::cout << "  Adjacency Matrix Space Complexity        : O(V^2)\n";
    std::cout << "    - Theoretical Matrix Memory Cells      : " << denseMatrixCells << " cells (Mostly Empty 0s)\n";
    std::cout << "    - Estimated RAM Required               : ~" << (estMatrixBytes / 1024.0) << " KB\n";
    std::cout << "  Adjacency List vs Matrix Efficiency      : "
              << std::fixed << std::setprecision(2)
              << (100.0 * (v + e) / denseMatrixCells) << "% of matrix size (Sparse Optimization)\n";
    std::cout << "-----------------------------------------------------------------------------------\n";
    std::cout << "  State-Augmented Dijkstra State Space     : |V| * |M| = " << (v * modes) << " expanded states\n";
    std::cout << "  Binary Min-Heap State Transitions        : |E| * |M| = " << (e * modes) << " relaxed edges\n";
    std::cout << "-----------------------------------------------------------------------------------\n";
    std::cout << "  VIVA EXPLANATION:\n";
    std::cout << "  - Because transit networks are sparse (|E| << |V|^2), an Adjacency List eliminates\n";
    std::cout << "    memory overhead and achieves optimal CPU cache locality during edge relaxations.\n";
    std::cout << "  - Relaxing neighbors takes O(deg(u)) instead of scanning all O(|V|) matrix entries.\n";
    std::cout << "===================================================================================\n\n";
}

void TransitProfiler::printReport(const BenchmarkPoint& point, std::ostream& out) {
    out << "\n+-----------------------------------------------------------------------+\n";
    out << "|         ALGORITHMIC PROFILING & BENCHMARK REPORT (C++17)             |\n";
    out << "+-----------------------------------------------------------------------+\n";
    out << "  Total Queries Profiled    : " << point.queryCount << "\n";
    out << "  Successful / Disconnected : " << point.successfulRoutes << " / " << point.failedRoutes << "\n";
    out << "  Total Elapsed Wall-Clock  : " << std::fixed << std::setprecision(5) << point.totalTimeSec << " s ("
        << (point.totalTimeSec * 1000.0) << " ms)\n";
    out << "  Throughput (QPS)          : " << std::fixed << std::setprecision(0) << point.throughputQPS << " queries/sec\n";
    out << "  ---------------------------------------------------------------------\n";
    out << "  LATENCY METRICS (Microseconds, \xCE\xBCs):\n";
    out << "    Minimum Latency         : " << std::fixed << std::setprecision(2) << point.minLatencyMicrosec << " \xCE\xBCs\n";
    out << "    Average (Mean) Latency  : " << std::fixed << std::setprecision(2) << point.avgLatencyMicrosec << " \xCE\xBCs\n";
    out << "    Median (50th Percentile): " << std::fixed << std::setprecision(2) << point.medianLatencyMicrosec << " \xCE\xBCs\n";
    out << "    95th Percentile (P95)   : " << std::fixed << std::setprecision(2) << point.p95LatencyMicrosec << " \xCE\xBCs\n";
    out << "    99th Percentile (P99)   : " << std::fixed << std::setprecision(2) << point.p99LatencyMicrosec << " \xCE\xBCs\n";
    out << "    Maximum Latency         : " << std::fixed << std::setprecision(2) << point.maxLatencyMicrosec << " \xCE\xBCs\n";
    out << "+-----------------------------------------------------------------------+\n\n";
}

void TransitProfiler::printScalingTable(const std::vector<BenchmarkPoint>& results, std::ostream& out) {
    out << "\n===================================================================================================\n";
    out << "                  THROUGHPUT & LATENCY SCALING BENCHMARK (N=100 to 25,000)                         \n";
    out << "===================================================================================================\n";
    out << std::left
        << std::setw(12) << "Queries (N)"
        << std::setw(16) << "Total Time (s)"
        << std::setw(18) << "Throughput (QPS)"
        << std::setw(14) << "Avg (\xCE\xBCs)"
        << std::setw(14) << "Med (\xCE\xBCs)"
        << std::setw(14) << "P95 (\xCE\xBCs)"
        << std::setw(14) << "P99 (\xCE\xBCs)\n";
    out << "---------------------------------------------------------------------------------------------------\n";

    for (const auto& b : results) {
        out << std::left
            << std::setw(12) << b.queryCount
            << std::fixed << std::setprecision(5) << std::setw(16) << b.totalTimeSec
            << std::setprecision(0) << std::setw(18) << b.throughputQPS
            << std::setprecision(2) << std::setw(14) << b.avgLatencyMicrosec
            << std::setprecision(2) << std::setw(14) << b.medianLatencyMicrosec
            << std::setprecision(2) << std::setw(14) << b.p95LatencyMicrosec
            << std::setprecision(2) << std::setw(14) << b.p99LatencyMicrosec
            << "\n";
    }
    out << "===================================================================================================\n\n";
}

bool TransitProfiler::exportToCSV(const std::vector<BenchmarkPoint>& results, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    file << "QueryCount,TotalTimeSec,ThroughputQPS,AvgLatencyMicrosec,MinLatencyMicrosec,MaxLatencyMicrosec,MedianLatencyMicrosec,P95LatencyMicrosec,P99LatencyMicrosec\n";
    for (const auto& b : results) {
        file << b.queryCount << ","
             << std::fixed << std::setprecision(6) << b.totalTimeSec << ","
             << std::fixed << std::setprecision(2) << b.throughputQPS << ","
             << std::fixed << std::setprecision(3) << b.avgLatencyMicrosec << ","
             << std::fixed << std::setprecision(3) << b.minLatencyMicrosec << ","
             << std::fixed << std::setprecision(3) << b.maxLatencyMicrosec << ","
             << std::fixed << std::setprecision(3) << b.medianLatencyMicrosec << ","
             << std::fixed << std::setprecision(3) << b.p95LatencyMicrosec << ","
             << std::fixed << std::setprecision(3) << b.p99LatencyMicrosec << "\n";
    }
    return true;
}
