#ifndef PROFILER_H
#define PROFILER_H

#include "Graph.h"
#include <vector>
#include <string>
#include <iostream>

using namespace std;

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

class TransitProfiler {
private:
    const MultiModalGraph& graph;

public:
    explicit TransitProfiler(const MultiModalGraph& g) : graph(g) {}

    BenchmarkPoint runSingleBenchmark(int queryCount, unsigned int seed = 42) const;
    vector<BenchmarkPoint> runScalingBenchmark(const vector<int>& batchSizes = {100, 500, 2000, 10000, 25000});

    void printSpaceComplexityAnalysis() const;
    static void printReport(const BenchmarkPoint& point, ostream& out = cout);
    static void printScalingTable(const vector<BenchmarkPoint>& results, ostream& out = cout);
    static bool exportToCSV(const vector<BenchmarkPoint>& results, const string& filepath = "benchmark_scaling.csv");
};

#endif // PROFILER_H
