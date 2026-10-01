import heapq
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Tuple
from src.models.graph import MultiModalGraph
from src.models.edge import Edge, TransportMode


@dataclass
class RouteLeg:
    """Represents an individual leg/segment of a multi-modal journey."""
    source: str
    target: str
    mode: TransportMode
    distance_km: float
    travel_time_min: float
    is_transfer_before: bool = False
    transfer_penalty_min: float = 0.0


@dataclass
class RouteResult:
    """Represents the complete optimal route solution found by Dijkstra."""
    origin: str
    destination: str
    total_time_min: float
    total_distance_km: float
    transfer_count: int
    legs: List[RouteLeg] = field(default_factory=list)
    path_nodes: List[str] = field(default_factory=list)

    def is_reachable(self) -> bool:
        return len(self.path_nodes) > 0


class MultiModalDijkstra:
    """
    State-Augmented Multi-Modal Dijkstra Algorithm with Mode Transfer Penalties.
    
    Why standard Dijkstra fails:
      Standard Dijkstra maintains state as (distance, vertex u). However, in a
      multi-modal transit network, arriving at vertex u via TRAIN vs arriving via BUS
      carries different future transition costs (platform transfer delay, ticket checks).
      
    State Space:
      State is expanded to: (u, current_mode) where:
        u in V (transit stops/stations)
        current_mode in {BUS, TRAIN, None}
      Total state space size: |V| * |M| (where |M| = 2 modes)
      
    Complexity:
      Time Complexity: O((|E| * |M|) * log(|V| * |M|)) using Binary Min-Heap (heapq).
      Space Complexity: O(|V| * |M| + |E|) for distance tables & predecessor pointers.
    """

    def __init__(self, graph: MultiModalGraph, default_transfer_penalty_min: float = 6.0) -> None:
        self.graph = graph
        self.transfer_penalty_min = default_transfer_penalty_min

    def find_shortest_path(
        self,
        origin_id: str,
        destination_id: str,
        custom_transfer_penalty: Optional[float] = None
    ) -> RouteResult:
        """
        Computes the time-optimal multi-modal path from origin to destination.
        
        Args:
            origin_id: Starting station/stop ID.
            destination_id: Destination station/stop ID.
            custom_transfer_penalty: Optional override for transfer delay in minutes.
            
        Returns:
            RouteResult containing total time, distance, transfers, and breakdown.
        """
        penalty = custom_transfer_penalty if custom_transfer_penalty is not None else self.transfer_penalty_min

        # Validate inputs
        if origin_id not in self.graph.nodes or destination_id not in self.graph.nodes:
            return RouteResult(origin=origin_id, destination=destination_id, total_time_min=0.0, total_distance_km=0.0, transfer_count=0)

        if origin_id == destination_id:
            return RouteResult(
                origin=origin_id,
                destination=destination_id,
                total_time_min=0.0,
                total_distance_km=0.0,
                transfer_count=0,
                path_nodes=[origin_id],
            )

        # Min-Heap Priority Queue: stores tuples of (cumulative_cost, node_id, mode_or_None)
        pq: List[Tuple[float, str, Optional[str]]] = []

        # Distance table: (node_id, mode_str) -> min_cost
        # Using string representation for mode in state keys ("BUS", "TRAIN", "START")
        distances: Dict[Tuple[str, Optional[str]], float] = {}

        # Predecessor map for path reconstruction:
        # (node_id, mode_str) -> (prev_node_id, prev_mode_str, edge_used, transfer_applied, transfer_delay)
        predecessors: Dict[
            Tuple[str, Optional[str]],
            Tuple[str, Optional[str], Edge, bool, float]
        ] = {}

        # Initial state: At origin with no prior transport mode
        initial_state = (origin_id, None)
        distances[initial_state] = 0.0
        heapq.heappush(pq, (0.0, origin_id, None))

        best_terminal_state: Optional[Tuple[str, Optional[str]]] = None
        min_dest_cost = float("inf")

        while pq:
            curr_cost, u, curr_mode_str = heapq.heappop(pq)
            curr_state = (u, curr_mode_str)

            # Pruning: if we found a strictly better cost to this state already, skip
            if curr_cost > distances.get(curr_state, float("inf")):
                continue

            # If we reached destination, check if this is the overall best cost
            if u == destination_id:
                if curr_cost < min_dest_cost:
                    min_dest_cost = curr_cost
                    best_terminal_state = curr_state
                # We can't immediately break because another mode state might arrive with lower cost,
                # but because heap is monotonic, first arrival is usually optimal unless equal.
                continue

            # Explore outgoing edges
            for edge in self.graph.get_neighbors(u):
                v = edge.target
                edge_mode_str = edge.mode.value
                next_state = (v, edge_mode_str)

                # Check if transfer penalty applies (switching between modes)
                is_transfer = False
                transfer_delay = 0.0

                if curr_mode_str is not None and curr_mode_str != edge_mode_str:
                    is_transfer = True
                    transfer_delay = penalty

                # Effective travel time considering current passenger congestion
                effective_edge_time = edge.get_effective_travel_time()
                new_cost = curr_cost + effective_edge_time + transfer_delay

                if new_cost < distances.get(next_state, float("inf")):
                    distances[next_state] = new_cost
                    predecessors[next_state] = (u, curr_mode_str, edge, is_transfer, transfer_delay)
                    heapq.heappush(pq, (new_cost, v, edge_mode_str))

        if best_terminal_state is None:
            # Destination unreachable
            return RouteResult(origin=origin_id, destination=destination_id, total_time_min=float("inf"), total_distance_km=0.0, transfer_count=0)

        # Reconstruct path and journey legs
        legs: List[RouteLeg] = []
        path_nodes: List[str] = [destination_id]
        curr_state = best_terminal_state
        total_distance = 0.0
        transfer_count = 0

        while curr_state in predecessors:
            prev_u, prev_mode_str, edge, is_transfer, transfer_delay = predecessors[curr_state]
            if is_transfer:
                transfer_count += 1

            leg = RouteLeg(
                source=edge.source,
                target=edge.target,
                mode=edge.mode,
                distance_km=edge.distance_km,
                travel_time_min=edge.get_effective_travel_time(),
                is_transfer_before=is_transfer,
                transfer_penalty_min=transfer_delay,
            )
            legs.append(leg)
            total_distance += edge.distance_km
            path_nodes.append(prev_u)
            curr_state = (prev_u, prev_mode_str)

        legs.reverse()
        path_nodes.reverse()

        return RouteResult(
            origin=origin_id,
            destination=destination_id,
            total_time_min=round(min_dest_cost, 2),
            total_distance_km=round(total_distance, 2),
            transfer_count=transfer_count,
            legs=legs,
            path_nodes=path_nodes,
        )
