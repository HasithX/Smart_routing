#ifndef PROFILER_H
#define PROFILER_H

#include "Graph.h"
#include <vector>
#include <string>
#include <iostream>

/**
 * @brief Holds benchmark metrics for query batch profiling.
 */
struct BenchmarkPoint {
    int queryCount;
    double totalTimeSec;
    double avgLatencyMicrosec;
    double throughputQPS;
    double minLatencyMicrosec;
    double maxLatencyMicrosec;
    double medianLatencyMicrosec;
    double p95LatencyMicrosec;
    double p99LatencyMicrosec;
    int successfulRoutes;
    int failedRoutes;

    BenchmarkPoint()
        : queryCount(0), totalTimeSec(0.0), avgLatencyMicrosec(0.0), throughputQPS(0.0),
          minLatencyMicrosec(0.0), maxLatencyMicrosec(0.0), medianLatencyMicrosec(0.0),
          p95LatencyMicrosec(0.0), p99LatencyMicrosec(0.0), successfulRoutes(0), failedRoutes(0) {}
};

/**
 * @brief Member 4 Algorithmic Profiler and Benchmarking Suite.
 */
class TransitProfiler {
private:
    const MultiModalGraph& graph;

public:
    explicit TransitProfiler(const MultiModalGraph& g) : graph(g) {}

    /**
     * @brief Profiles a single batch of routing queries with microsecond resolution.
     */
    BenchmarkPoint runSingleBenchmark(int queryCount, unsigned int seed = 42) const;

    /**
     * @brief Runs scaling benchmark across multiple query load sizes (e.g. 100 to 25,000 queries).
     */
    std::vector<BenchmarkPoint> runScalingBenchmark(const std::vector<int>& batchSizes = {100, 500, 2000, 10000, 25000});

    /**
     * @brief Displays theoretical O(V+E) vs O(V^2) space complexity analysis for UCSC Viva.
     */
    void printSpaceComplexityAnalysis() const;

    /**
     * @brief Formats and prints a detailed statistical benchmark card to console.
     */
    static void printReport(const BenchmarkPoint& point, std::ostream& out = std::cout);

    /**
     * @brief Displays an ASCII comparative scaling table across different query loads.
     */
    static void printScalingTable(const std::vector<BenchmarkPoint>& results, std::ostream& out = std::cout);

    /**
     * @brief Exports benchmark scaling data to a CSV file.
     */
    static bool exportToCSV(const std::vector<BenchmarkPoint>& results, const std::string& filepath = "benchmark_scaling.csv");
};

#endif // PROFILER_H
