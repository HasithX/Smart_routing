import unittest
from src.models.location import Location, NodeType
from src.models.edge import Edge, TransportMode
from src.models.graph import MultiModalGraph
from src.algorithms.dijkstra import MultiModalDijkstra


class TestMultiModalRouting(unittest.TestCase):

    def setUp(self):
        """
        Creates a test network:
          A (Train) -> B (Transfer Hub) -> C (Bus)
          Direct Bus: A -> C (slower road)
        """
        self.graph = MultiModalGraph()
        self.graph.add_node(Location("A", "Station A", NodeType.TRAIN_STATION, "Suburbs"))
        self.graph.add_node(Location("B", "Hub B", NodeType.INTERCHANGE_HUB, "Midtown"))
        self.graph.add_node(Location("C", "Stop C", NodeType.BUS_STOP, "Downtown"))

        # Train: A -> B takes 10 mins
        self.graph.add_edge(Edge("T1", "A", "B", TransportMode.TRAIN, 10.0, 10.0, 500), bidirectional=False)
        # Bus: B -> C takes 5 mins
        self.graph.add_edge(Edge("B1", "B", "C", TransportMode.BUS, 3.0, 5.0, 80), bidirectional=False)
        # Direct Bus: A -> C takes 25 mins
        self.graph.add_edge(Edge("B2", "A", "C", TransportMode.BUS, 15.0, 25.0, 80), bidirectional=False)

    def test_transfer_penalty_applied(self):
        router = MultiModalDijkstra(self.graph, default_transfer_penalty_min=6.0)
        result = router.find_shortest_path("A", "C")

        # Path via B: Train(10) + Transfer(6) + Bus(5) = 21 mins
        # Direct Bus path: Bus(25) = 25 mins
        # Optimal should be via B with time = 21 mins and transfer_count = 1
        self.assertTrue(result.is_reachable())
        self.assertEqual(result.path_nodes, ["A", "B", "C"])
        self.assertEqual(result.total_time_min, 21.0)
        self.assertEqual(result.transfer_count, 1)

    def test_high_penalty_forces_direct_mode(self):
        # If transfer penalty is very high (e.g. 20 mins), direct bus (25 mins) should win over 10 + 20 + 5 = 35 mins!
        router = MultiModalDijkstra(self.graph, default_transfer_penalty_min=20.0)
        result = router.find_shortest_path("A", "C")

        self.assertTrue(result.is_reachable())
        self.assertEqual(result.path_nodes, ["A", "C"])
        self.assertEqual(result.total_time_min, 25.0)
        self.assertEqual(result.transfer_count, 0)


if __name__ == "__main__":
    unittest.main()
