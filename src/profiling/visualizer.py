from typing import Optional, List
from src.models.graph import MultiModalGraph
from src.models.location import NodeType
from src.models.edge import TransportMode
from src.algorithms.dijkstra import RouteResult


class TransitVisualizer:
    """
    Renders topological multi-modal network visualizations using NetworkX and Matplotlib.
    Distinguishes Train tracks (solid Crimson), Bus routes (dashed SteelBlue),
    and Interchange Hubs (Gold stars/diamonds).
    """

    def __init__(self, graph: MultiModalGraph) -> None:
        self.graph = graph

    def plot_transit_network(
        self,
        highlight_route: Optional[RouteResult] = None,
        save_path: str = "city_transit_network.png",
        show_plot: bool = False
    ) -> str:
        """
        Plots the full multi-modal smart city transit graph.
        Can optionally overlay a calculated passenger route path.
        """
        try:
            import networkx as nx
            import matplotlib.pyplot as plt
        except ImportError:
            return "networkx or matplotlib not installed. Run 'pip install -r requirements.txt'."

        G = nx.MultiDiGraph()

        # Add Nodes with coordinates
        pos = {}
        node_colors = []
        node_sizes = []

        for nid, loc in self.graph.nodes.items():
            G.add_node(nid, label=loc.name, zone=loc.zone)
            pos[nid] = (loc.x, loc.y)

            if loc.node_type == NodeType.INTERCHANGE_HUB:
                node_colors.append("#F59E0B")  # Amber / Gold
                node_sizes.append(600)
            elif loc.node_type == NodeType.TRAIN_STATION:
                node_colors.append("#EF4444")  # Red
                node_sizes.append(450)
            else:
                node_colors.append("#3B82F6")  # Blue
                node_sizes.append(300)

        # Classify Edges by mode
        train_edges = []
        bus_edges = []

        for u, edges in self.graph.adjacency_list.items():
            for e in edges:
                G.add_edge(e.source, e.target, mode=e.mode.value, time=e.base_time_min)
                if e.mode == TransportMode.TRAIN:
                    train_edges.append((e.source, e.target))
                else:
                    bus_edges.append((e.source, e.target))

        plt.figure(figsize=(13, 9), facecolor="#0F172A")
        ax = plt.gca()
        ax.set_facecolor("#0F172A")

        # Draw Bus lines (Blue / dashed)
        nx.draw_networkx_edges(
            G, pos,
            edgelist=bus_edges,
            edge_color="#60A5FA",
            width=1.8,
            style="dashed",
            alpha=0.6,
            arrows=False
        )

        # Draw Train lines (Red / solid)
        nx.draw_networkx_edges(
            G, pos,
            edgelist=train_edges,
            edge_color="#F87171",
            width=3.5,
            style="solid",
            alpha=0.85,
            arrows=False
        )

        # Highlight optimal route if provided
        if highlight_route and highlight_route.is_reachable():
            route_edges = [
                (leg.source, leg.target) for leg in highlight_route.legs
            ]
            nx.draw_networkx_edges(
                G, pos,
                edgelist=route_edges,
                edge_color="#10B981",  # Emerald Green
                width=5.0,
                arrows=True,
                arrowsize=18,
                arrowstyle="-|>",
            )

        # Draw Nodes
        nx.draw_networkx_nodes(
            G, pos,
            node_color=node_colors,
            node_size=node_sizes,
            edgecolors="#FFFFFF",
            linewidths=1.5
        )

        # Draw Labels
        labels = {nid: f"{nid}\n{self.graph.nodes[nid].name.replace('_', ' ')}" for nid in G.nodes()}
        nx.draw_networkx_labels(
            G, pos,
            labels=labels,
            font_size=8,
            font_color="#F8FAFC",
            font_weight="bold"
        )

        # Legend & Title
        plt.title(
            "Smart City Multi-Modal Public Transit Network (Colombo Metro)\n"
            "--- Train Tracks (Solid Red)  |  - - - Bus Routes (Dashed Blue)  |  [#] Multi-Modal Interchange Hubs",
            color="#FFFFFF",
            fontsize=12,
            pad=15
        )
        plt.axis("off")
        plt.tight_layout()
        plt.savefig(save_path, dpi=200, bbox_inches="tight", facecolor="#0F172A")

        if show_plot:
            plt.show()

        plt.close()
        return f"Map saved successfully to '{save_path}'"
