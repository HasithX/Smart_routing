#ifndef DEMANDSIMULATOR_H
#define DEMANDSIMULATOR_H

#include "Graph.h"
#include "Dijkstra.h"
#include <string>
#include <vector>

using namespace std;

enum class TimeWindow {
    MORNING_PEAK,
    MIDDAY_OFF_PEAK,
    EVENING_PEAK,
    NIGHT_LOW
};

string timeWindowToString(TimeWindow tw);

struct PassengerTrip {
    string passengerId;
    string originId;
    string destinationId;
    string departureTime;
    TimeWindow window;
};

struct CongestedEdgeSummary {
    string routeId;
    string corridor;
    string mode;
    int flow;
    int capacity;
    double vcRatio;
    double effectiveTime;
    double baseTime;
};

struct SimulationResult {
    string timeWindowLabel;
    int simulatedCount;
    int routedCount;
    int unreachableCount;
    double avgTravelTimeMin;
    double avgTransfersPerTrip;
    vector<CongestedEdgeSummary> topCongestedEdges;
};

class DemandSimulator {
private:
    MultiModalGraph& graph;

public:
    explicit DemandSimulator(MultiModalGraph& g) : graph(g) {}

    vector<PassengerTrip> generateTrips(
        TimeWindow window,
        int totalPassengers,
        unsigned int seed = 42
    );

    SimulationResult simulateTimeWindow(
        TimeWindow window,
        int totalPassengers = 300,
        bool applyFeedback = true
    );

    vector<int> generate24HourDemand(int baseDemand);
    void simulateDisruption(const string& routeId);
};

#endif // DEMANDSIMULATOR_H
