from typing import Optional, List
from src.models.graph import MultiModalGraph
from src.algorithms.dijkstra import MultiModalDijkstra, RouteResult, RouteLeg


class RoutePlanner:
    """
    High-level transit routing orchestrator.
    Maps to the RoutePlanner interface (e.g. routeplanner.h in C architectures).
    Translates raw algorithmic paths into clean, human-readable city itineraries.
    """

    def __init__(self, graph: MultiModalGraph, default_transfer_penalty: float = 6.0) -> None:
        self.graph = graph
        self.engine = MultiModalDijkstra(graph, default_transfer_penalty_min=default_transfer_penalty)

    def plan_journey(self, origin_id: str, destination_id: str) -> RouteResult:
        """Finds the optimal journey from origin to destination."""
        return self.engine.find_shortest_path(origin_id, destination_id)

    def format_itinerary(self, result: RouteResult) -> str:
        """Formats the journey route result into a structured text itinerary."""
        if not result.is_reachable():
            return f"No public transit route found between {result.origin} and {result.destination}."

        orig_node = self.graph.get_node(result.origin)
        dest_node = self.graph.get_node(result.destination)
        orig_name = orig_node.name if orig_node else result.origin
        dest_name = dest_node.name if dest_node else result.destination

        lines: List[str] = [
            "=" * 65,
            f" SMART CITY TRANSIT ITINERARY: {orig_name} -> {dest_name}",
            "=" * 65,
            f" Total Travel Time : {result.total_time_min} mins",
            f" Total Distance    : {result.total_distance_km} km",
            f" Number of Transfers: {result.transfer_count}",
            "-" * 65,
            " Step-by-step directions:",
        ]

        step_num = 1
        for leg in result.legs:
            src_node = self.graph.get_node(leg.source)
            tgt_node = self.graph.get_node(leg.target)
            src_name = src_node.name if src_node else leg.source
            tgt_name = tgt_node.name if tgt_node else leg.target

            if leg.is_transfer_before:
                lines.append(
                    f"   [{step_num}] TRANSFER: Walk/Change platforms at '{src_name}' "
                    f"(+{leg.transfer_penalty_min} min penalty)"
                )
                step_num += 1

            mode_symbol = "🚆 Train" if leg.mode.value == "TRAIN" else "🚌 Bus"
            lines.append(
                f"   [{step_num}] {mode_symbol} from '{src_name}' to '{tgt_name}' "
                f"({leg.distance_km} km | {leg.travel_time_min} mins)"
            )
            step_num += 1

        lines.append("=" * 65)
        return "\n".join(lines)
