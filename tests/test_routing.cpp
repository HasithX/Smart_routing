#include "Graph.h"
#include "Dijkstra.h"
#include <iostream>
#include <cassert>

void testMultiModalRouting() {
    MultiModalGraph graph;
    graph.addNode(Location("A", "Station A", NodeType::TRAIN_STATION, "Suburbs", 0, 0));
    graph.addNode(Location("B", "Hub B", NodeType::INTERCHANGE_HUB, "Midtown", 10, 0));
    graph.addNode(Location("C", "Stop C", NodeType::BUS_STOP, "Downtown", 20, 0));

    // Train A -> B: 10 mins
    graph.addEdge(Edge("T1", "A", "B", TransportMode::TRAIN, 10.0, 10.0, 500), false);
    // Bus B -> C: 5 mins
    graph.addEdge(Edge("B1", "B", "C", TransportMode::BUS, 3.0, 5.0, 80), false);
    // Direct Bus A -> C: 25 mins
    graph.addEdge(Edge("B2", "A", "C", TransportMode::BUS, 15.0, 25.0, 80), false);

    // Test with default transfer penalty = 6 mins
    // Route via B: Train(10) + Transfer(6) + Bus(5) = 21 mins (Wins over 25 mins)
    MultiModalDijkstra router(graph, 6.0);
    RouteResult res = router.findShortestPath("A", "C");

    assert(res.isReachable());
    assert(res.pathNodes.size() == 3);
    assert(res.pathNodes[0] == "A" && res.pathNodes[1] == "B" && res.pathNodes[2] == "C");
    assert(res.transferCount == 1);
    assert(res.totalTimeMin == 21.0);

    // Test with high transfer penalty = 20 mins
    // Route via B: Train(10) + Transfer(20) + Bus(5) = 35 mins
    // Direct Bus: 25 mins (Wins over 35 mins)
    MultiModalDijkstra highPenaltyRouter(graph, 20.0);
    RouteResult resDirect = highPenaltyRouter.findShortestPath("A", "C");

    assert(resDirect.isReachable());
    assert(resDirect.pathNodes.size() == 2);
    assert(resDirect.pathNodes[0] == "A" && resDirect.pathNodes[1] == "C");
    assert(resDirect.transferCount == 0);
    assert(resDirect.totalTimeMin == 25.0);

    // Test Hasitha's Multi-Criteria Pareto Optimization:
    // 1. FASTEST_TIME -> Route A -> B -> C (21 mins, 1 transfer)
    RouteResult optFast = router.findOptimalPath("A", "C", RoutingPreference::FASTEST_TIME);
    assert(optFast.isReachable());
    assert(optFast.pathNodes.size() == 3);
    assert(optFast.transferCount == 1);
    assert(optFast.totalTimeMin == 21.0);

    // 2. MINIMUM_TRANSFERS -> Direct Bus A -> C (25 mins, 0 transfers)
    RouteResult optDirect = router.findOptimalPath("A", "C", RoutingPreference::MINIMUM_TRANSFERS);
    assert(optDirect.isReachable());
    assert(optDirect.pathNodes.size() == 2);
    assert(optDirect.transferCount == 0);
    assert(optDirect.totalTimeMin == 25.0);

    std::cout << "[PASS] All Multi-Modal Dijkstra & Pareto Multi-Criteria Routing Unit Tests PASSED!\n";
}

int main() {
    testMultiModalRouting();
    return 0;
}
