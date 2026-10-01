import unittest
from src.models.location import Location, NodeType
from src.models.edge import Edge, TransportMode
from src.models.graph import MultiModalGraph


class TestMultiModalGraph(unittest.TestCase):

    def setUp(self):
        self.graph = MultiModalGraph()
        self.n1 = Location(id="N1", name="Stop A", node_type=NodeType.BUS_STOP, zone="Residential")
        self.n2 = Location(id="N2", name="Hub B", node_type=NodeType.INTERCHANGE_HUB, zone="Commercial")
        self.graph.add_node(self.n1)
        self.graph.add_node(self.n2)

    def test_add_nodes_and_edges(self):
        edge = Edge(
            route_id="R1",
            source="N1",
            target="N2",
            mode=TransportMode.BUS,
            distance_km=5.0,
            base_time_min=10.0,
            capacity=100
        )
        self.graph.add_edge(edge, bidirectional=True)

        stats = self.graph.get_stats()
        self.assertEqual(stats["vertex_count"], 2)
        self.assertEqual(stats["total_directed_edges"], 2)
        self.assertEqual(stats["bus_directed_edges"], 2)

    def test_adjacency_list_neighbors(self):
        edge = Edge(
            route_id="R1",
            source="N1",
            target="N2",
            mode=TransportMode.TRAIN,
            distance_km=8.0,
            base_time_min=8.0,
            capacity=200
        )
        self.graph.add_edge(edge, bidirectional=False)
        neighbors = self.graph.get_neighbors("N1")
        self.assertEqual(len(neighbors), 1)
        self.assertEqual(neighbors[0].target, "N2")
        self.assertEqual(neighbors[0].mode, TransportMode.TRAIN)


if __name__ == "__main__":
    unittest.main()
