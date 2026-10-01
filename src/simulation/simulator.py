from typing import List, Dict, Any
from src.models.graph import MultiModalGraph
from src.algorithms.dijkstra import MultiModalDijkstra, RouteResult
from src.simulation.demand_matrix import PassengerTrip, TimeWindow, DemandMatrix


class TransitSimulator:
    """
    Simulates urban transit traffic dynamics by loading passenger batches onto the graph,
    updating edge flows, and recording congested corridors.
    """

    def __init__(self, graph: MultiModalGraph) -> None:
        self.graph = graph
        self.router = MultiModalDijkstra(graph)

    def simulate_time_window(
        self,
        time_window: TimeWindow,
        passenger_count: int = 300,
        apply_congestion_feedback: bool = True
    ) -> Dict[str, Any]:
        """
        Simulates passenger trips for a designated time window.
        
        Returns:
            Dictionary with aggregated statistics (average trip time, transfer counts,
            most congested routes, edge utilization).
        """
        # Reset previous congestion if fresh run
        self.graph.reset_all_congestion()

        generator = DemandMatrix(self.graph)
        trips = generator.generate_trips(time_window=time_window, total_passengers=passenger_count)

        successful_routes: List[RouteResult] = []
        unreachable_count = 0
        total_travel_time = 0.0
        total_transfers = 0

        # Step 1: Route each passenger through the network
        for trip in trips:
            res = self.router.find_shortest_path(trip.origin_id, trip.destination_id)
            if res.is_reachable():
                successful_routes.append(res)
                total_travel_time += res.total_time_min
                total_transfers += res.transfer_count

                # Update passenger flow on edges traversed
                if apply_congestion_feedback:
                    for leg in res.legs:
                        for edge in self.graph.get_neighbors(leg.source):
                            if edge.target == leg.target and edge.mode == leg.mode:
                                edge.add_flow(1)
            else:
                unreachable_count += 1

        total_routed = len(successful_routes)
        avg_travel_time = round(total_travel_time / total_routed, 2) if total_routed > 0 else 0.0
        avg_transfers = round(total_transfers / total_routed, 2) if total_routed > 0 else 0.0

        # Identify top congested edges
        all_edges = [edge for edge_list in self.graph.adjacency_list.values() for edge in edge_list]
        congested_edges = sorted(
            all_edges,
            key=lambda e: (e.current_flow / e.capacity) if e.capacity > 0 else 0,
            reverse=True
        )[:5]

        congested_summary = [
            {
                "route_id": e.route_id,
                "corridor": f"{e.source} -> {e.target}",
                "mode": e.mode.value,
                "flow": e.current_flow,
                "capacity": e.capacity,
                "v_c_ratio": round(e.current_flow / e.capacity, 2) if e.capacity > 0 else 0,
                "effective_time": e.get_effective_travel_time(),
                "base_time": e.base_time_min,
            }
            for e in congested_edges
        ]

        return {
            "time_window": time_window.value,
            "passengers_simulated": len(trips),
            "passengers_routed": total_routed,
            "unreachable": unreachable_count,
            "avg_travel_time_min": avg_travel_time,
            "avg_transfers_per_trip": avg_transfers,
            "top_congested_edges": congested_summary,
        }
