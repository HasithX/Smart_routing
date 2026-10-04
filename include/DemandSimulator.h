#ifndef DEMANDSIMULATOR_H
#define DEMANDSIMULATOR_H

#include "Graph.h"
#include "Dijkstra.h"
#include <string>
#include <vector>

enum class TimeWindow {
    MORNING_PEAK,
    MIDDAY_OFF_PEAK,
    EVENING_PEAK,
    NIGHT_LOW
};

std::string timeWindowToString(TimeWindow tw);

struct PassengerTrip {
    std::string passengerId;
    std::string originId;
    std::string destinationId;
    std::string departureTime;
    TimeWindow window;
};

struct CongestedEdgeSummary {
    std::string routeId;
    std::string corridor;
    std::string mode;
    int flow;
    int capacity;
    double vcRatio;
    double effectiveTime;
    double baseTime;
};

struct SimulationResult {
    std::string timeWindowLabel;
    int simulatedCount;
    int routedCount;
    int unreachableCount;
    double avgTravelTimeMin;
    double avgTransfersPerTrip;
    std::vector<CongestedEdgeSummary> topCongestedEdges;
};

class DemandSimulator {
private:
    MultiModalGraph& graph;

public:
    explicit DemandSimulator(MultiModalGraph& g) : graph(g) {}

    // Generate passenger trips according to time-of-day demand patterns.
    std::vector<PassengerTrip> generateTrips(
        TimeWindow window,
        int totalPassengers,
        unsigned int seed = 42
    );

    // Run one time-window simulation and feed passenger flow back into edges.
    SimulationResult simulateTimeWindow(
        TimeWindow window,
        int totalPassengers = 300,
        bool applyFeedback = true
    );

    // Generate the expected passenger demand for every hour of a 24-hour day.
    std::vector<int> generate24HourDemand(int baseDemand);

    // Simulate a route/track failure and measure alternate-route impact.
    void simulateDisruption(const std::string& routeId);
};

#endif // DEMANDSIMULATOR_H
