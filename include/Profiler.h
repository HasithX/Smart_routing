#ifndef PROFILER_H
#define PROFILER_H

#include "Graph.h"
#include <vector>
#include <string>

struct BenchmarkPoint {
    int queryCount;
    double totalTimeSec;
    double avgLatencyMicrosec;
    double throughputQPS;
};

class TransitProfiler {
private:
    const MultiModalGraph& graph;

public:
    explicit TransitProfiler(const MultiModalGraph& g) : graph(g) {}

    std::vector<BenchmarkPoint> runScalingBenchmark(const std::vector<int>& batchSizes = {100, 500, 2000, 10000, 25000});
    void printSpaceComplexityAnalysis() const;
};

#endif // PROFILER_H
