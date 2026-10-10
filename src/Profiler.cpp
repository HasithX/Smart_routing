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

using namespace std;

BenchmarkPoint TransitProfiler::runSingleBenchmark(int queryCount, unsigned int seed) const {
    BenchmarkPoint result;
    result.queryCount = queryCount;

    if (graph.nodes.size() < 2 || queryCount <= 0) {
        return result;
    }

    vector<string> nodeIds;
    nodeIds.reserve(graph.nodes.size());
    for (const auto& pair : graph.nodes) {
        nodeIds.push_back(pair.first);
    }

    mt19937 rng(seed);
    uniform_int_distribution<size_t> dist(0, nodeIds.size() - 1);

    // generate test queries
    vector<pair<string, string>> queries;
    queries.reserve(queryCount);
    for (int i = 0; i < queryCount; ++i) {
        string u = nodeIds[dist(rng)];
        string v = nodeIds[dist(rng)];
        while (v == u) {
            v = nodeIds[dist(rng)];
        }
        queries.emplace_back(u, v);
    }

    MultiModalDijkstra router(graph);

    // cache warmup
    int warmupCount = min(50, queryCount);
    for (int i = 0; i < warmupCount; ++i) {
        volatile auto warm = router.findShortestPath(queries[i].first, queries[i].second);
        (void)warm;
    }

    vector<double> latenciesUs;
    latenciesUs.reserve(queryCount);

    auto startOverall = chrono::high_resolution_clock::now();

    for (int i = 0; i < queryCount; ++i) {
        auto qStart = chrono::high_resolution_clock::now();
        RouteResult res = router.findShortestPath(queries[i].first, queries[i].second);
        auto qEnd = chrono::high_resolution_clock::now();

        double elapsedUs = chrono::duration<double, micro>(qEnd - qStart).count();
        latenciesUs.push_back(elapsedUs);

        if (res.isReachable()) {
            result.successfulRoutes++;
        } else {
            result.failedRoutes++;
        }
    }

    auto endOverall = chrono::high_resolution_clock::now();
    result.totalTimeSec = chrono::duration<double>(endOverall - startOverall).count();

    if (!latenciesUs.empty()) {
        sort(latenciesUs.begin(), latenciesUs.end());

        double sum = accumulate(latenciesUs.begin(), latenciesUs.end(), 0.0);
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

vector<BenchmarkPoint> TransitProfiler::runScalingBenchmark(const vector<int>& batchSizes) {
    vector<BenchmarkPoint> results;
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
    int modes = 2;

    int denseMatrixCells = v * v;
    size_t estAdjListBytes = (v * sizeof(vector<Edge>)) + (e * sizeof(Edge)) + (v * sizeof(Location));
    size_t estMatrixBytes = static_cast<size_t>(denseMatrixCells) * sizeof(double);

    cout << "\n===================================================================================\n";
    cout << "                   DATA STRUCTURE SPACE COMPLEXITY PROFILING                       \n";
    cout << "===================================================================================\n";
    cout << "  Vertices V (Stations & Interchange Hubs) : " << v << "\n";
    cout << "  Directed Edges E (Roads & Rail Links)    : " << e << "\n";
    cout << "  Active Transit Modes |M|                 : " << modes << " (Bus, Train)\n";
    cout << "  Graph Density (|E| / (|V|*(|V|-1)))      : "
         << fixed << setprecision(4)
         << ((v > 1) ? (static_cast<double>(e) / (v * (v - 1))) : 0.0) << "\n";
    cout << "-----------------------------------------------------------------------------------\n";
    cout << "  Adjacency List Space Complexity          : O(V + E)\n";
    cout << "    - Theoretical Memory Records           : " << (v + e) << " records\n";
    cout << "    - Estimated RAM Allocation             : ~" << (estAdjListBytes / 1024.0) << " KB\n";
    cout << "  Adjacency Matrix Space Complexity        : O(V^2)\n";
    cout << "    - Theoretical Matrix Memory Cells      : " << denseMatrixCells << " cells\n";
    cout << "    - Estimated RAM Required               : ~" << (estMatrixBytes / 1024.0) << " KB\n";
    cout << "  Adjacency List vs Matrix Efficiency      : "
         << fixed << setprecision(2)
         << (100.0 * (v + e) / denseMatrixCells) << "% of matrix size\n";
    cout << "-----------------------------------------------------------------------------------\n";
    cout << "  State-Augmented Dijkstra State Space     : |V| * |M| = " << (v * modes) << " states\n";
    cout << "  Binary Min-Heap State Transitions        : |E| * |M| = " << (e * modes) << " edges\n";
    cout << "-----------------------------------------------------------------------------------\n";
    cout << "  Summary:\n";
    cout << "  - Because transit networks are sparse (|E| << |V|^2), an Adjacency List eliminates\n";
    cout << "    memory overhead and achieves optimal CPU cache locality during edge relaxations.\n";
    cout << "  - Relaxing neighbors takes O(deg(u)) instead of scanning all O(|V|) matrix entries.\n";
    cout << "===================================================================================\n\n";
}

void TransitProfiler::printReport(const BenchmarkPoint& point, ostream& out) {
    out << "\n+-----------------------------------------------------------------------+\n";
    out << "|                     BENCHMARK PROFILE REPORT                          |\n";
    out << "+-----------------------------------------------------------------------+\n";
    out << "  Total Queries Profiled    : " << point.queryCount << "\n";
    out << "  Successful / Disconnected : " << point.successfulRoutes << " / " << point.failedRoutes << "\n";
    out << "  Total Elapsed Wall-Clock  : " << fixed << setprecision(5) << point.totalTimeSec << " s ("
        << (point.totalTimeSec * 1000.0) << " ms)\n";
    out << "  Throughput (QPS)          : " << fixed << setprecision(0) << point.throughputQPS << " queries/sec\n";
    out << "  ---------------------------------------------------------------------\n";
    out << "  LATENCY METRICS (Microseconds, \xCE\xBCs):\n";
    out << "    Minimum Latency         : " << fixed << setprecision(2) << point.minLatencyMicrosec << " \xCE\xBCs\n";
    out << "    Average (Mean) Latency  : " << fixed << setprecision(2) << point.avgLatencyMicrosec << " \xCE\xBCs\n";
    out << "    Median (50th Percentile): " << fixed << setprecision(2) << point.medianLatencyMicrosec << " \xCE\xBCs\n";
    out << "    95th Percentile (P95)   : " << fixed << setprecision(2) << point.p95LatencyMicrosec << " \xCE\xBCs\n";
    out << "    99th Percentile (P99)   : " << fixed << setprecision(2) << point.p99LatencyMicrosec << " \xCE\xBCs\n";
    out << "    Maximum Latency         : " << fixed << setprecision(2) << point.maxLatencyMicrosec << " \xCE\xBCs\n";
    out << "+-----------------------------------------------------------------------+\n\n";
}

void TransitProfiler::printScalingTable(const vector<BenchmarkPoint>& results, ostream& out) {
    out << "\n===================================================================================================\n";
    out << "                  THROUGHPUT & LATENCY SCALING BENCHMARK (N=100 to 25,000)                         \n";
    out << "===================================================================================================\n";
    out << left
        << setw(12) << "Queries (N)"
        << setw(16) << "Total Time (s)"
        << setw(18) << "Throughput (QPS)"
        << setw(14) << "Avg (\xCE\xBCs)"
        << setw(14) << "Med (\xCE\xBCs)"
        << setw(14) << "P95 (\xCE\xBCs)"
        << setw(14) << "P99 (\xCE\xBCs)\n";
    out << "---------------------------------------------------------------------------------------------------\n";

    for (const auto& b : results) {
        out << left
            << setw(12) << b.queryCount
            << fixed << setprecision(5) << setw(16) << b.totalTimeSec
            << setprecision(0) << setw(18) << b.throughputQPS
            << setprecision(2) << setw(14) << b.avgLatencyMicrosec
            << setprecision(2) << setw(14) << b.medianLatencyMicrosec
            << setprecision(2) << setw(14) << b.p95LatencyMicrosec
            << setprecision(2) << setw(14) << b.p99LatencyMicrosec
            << "\n";
    }
    out << "===================================================================================================\n\n";
}

bool TransitProfiler::exportToCSV(const vector<BenchmarkPoint>& results, const string& filepath) {
    ofstream file(filepath);
    if (!file.is_open()) return false;

    file << "QueryCount,TotalTimeSec,ThroughputQPS,AvgLatencyMicrosec,MinLatencyMicrosec,MaxLatencyMicrosec,MedianLatencyMicrosec,P95LatencyMicrosec,P99LatencyMicrosec\n";
    for (const auto& b : results) {
        file << b.queryCount << ","
             << fixed << setprecision(6) << b.totalTimeSec << ","
             << fixed << setprecision(2) << b.throughputQPS << ","
             << fixed << setprecision(3) << b.avgLatencyMicrosec << ","
             << fixed << setprecision(3) << b.minLatencyMicrosec << ","
             << fixed << setprecision(3) << b.maxLatencyMicrosec << ","
             << fixed << setprecision(3) << b.medianLatencyMicrosec << ","
             << fixed << setprecision(3) << b.p95LatencyMicrosec << ","
             << fixed << setprecision(3) << b.p99LatencyMicrosec << "\n";
    }
    return true;
}
