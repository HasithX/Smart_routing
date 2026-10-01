import csv
from typing import Dict, List, Optional
from src.models.location import Location, NodeType
from src.models.edge import Edge, TransportMode


class MultiModalGraph:
    """
    Weighted Directed/Undirected Multi-Modal Transit Graph using an Adjacency List.
    Space Complexity: O(V + E) where:
      - V = Total number of transit locations/stations
      - E = Total number of directed transit routes (Bus roads and Train tracks)
    """

    def __init__(self) -> None:
        # Vertex Map: Node ID -> Location Object
        self.nodes: Dict[str, Location] = {}
        
        # Adjacency List: Node ID -> List of Outgoing Edges
        self.adjacency_list: Dict[str, List[Edge]] = {}

    def add_node(self, node: Location) -> None:
        """Adds a transit node to the graph in O(1) time."""
        if node.id not in self.nodes:
            self.nodes[node.id] = node
            self.adjacency_list[node.id] = []

    def add_edge(self, edge: Edge, bidirectional: bool = True) -> None:
        """
        Adds a directed transit edge to the adjacency list in O(1) amortized time.
        If bidirectional=True, also adds the reverse edge.
        """
        if edge.source not in self.nodes or edge.target not in self.nodes:
            raise ValueError(f"Both endpoints ({edge.source}, {edge.target}) must exist in graph.")

        self.adjacency_list[edge.source].append(edge)

        if bidirectional:
            reverse_edge = Edge(
                route_id=f"{edge.route_id}_REV",
                source=edge.target,
                target=edge.source,
                mode=edge.mode,
                distance_km=edge.distance_km,
                base_time_min=edge.base_time_min,
                capacity=edge.capacity,
                current_flow=edge.current_flow,
            )
            self.adjacency_list[edge.target].append(reverse_edge)

    def get_neighbors(self, node_id: str) -> List[Edge]:
        """Returns all outgoing edges from node_id in O(1) lookup time."""
        return self.adjacency_list.get(node_id, [])

    def get_node(self, node_id: str) -> Optional[Location]:
        return self.nodes.get(node_id)

    def load_from_csv(self, stations_path: str, routes_path: str) -> None:
        """Populates the graph from CSV data files."""
        # Load Stations
        with open(stations_path, mode="r", encoding="utf-8") as f:
            reader = csv.DictReader(f)
            for row in reader:
                loc = Location(
                    id=row["id"].strip(),
                    name=row["name"].strip(),
                    node_type=NodeType.from_str(row["type"]),
                    zone=row.get("zone", "General").strip(),
                    x=float(row.get("x", 0.0)),
                    y=float(row.get("y", 0.0)),
                )
                self.add_node(loc)

        # Load Routes
        with open(routes_path, mode="r", encoding="utf-8") as f:
            reader = csv.DictReader(f)
            for row in reader:
                is_bidi = row.get("bidirectional", "true").strip().lower() == "true"
                edge = Edge(
                    route_id=row["route_id"].strip(),
                    source=row["source_id"].strip(),
                    target=row["target_id"].strip(),
                    mode=TransportMode.from_str(row["mode"]),
                    distance_km=float(row["distance_km"]),
                    base_time_min=float(row["base_time_min"]),
                    capacity=int(row.get("capacity", 100)),
                )
                self.add_edge(edge, bidirectional=is_bidi)

    def reset_all_congestion(self) -> None:
        """Resets passenger flow counts across all edges to baseline."""
        for edges in self.adjacency_list.values():
            for edge in edges:
                edge.reset_flow()

    def get_stats(self) -> dict:
        total_edges = sum(len(edges) for edges in self.adjacency_list.values())
        bus_edges = sum(1 for edges in self.adjacency_list.values() for e in edges if e.mode == TransportMode.BUS)
        train_edges = sum(1 for edges in self.adjacency_list.values() for e in edges if e.mode == TransportMode.TRAIN)
        interchange_count = sum(1 for n in self.nodes.values() if n.is_interchange)

        return {
            "vertex_count": len(self.nodes),
            "total_directed_edges": total_edges,
            "bus_directed_edges": bus_edges,
            "train_directed_edges": train_edges,
            "interchange_hubs": interchange_count,
        }
