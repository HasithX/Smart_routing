import cProfile
import pstats
import io
import time
import sys
from typing import Dict, Any, List
from src.models.graph import MultiModalGraph
from src.algorithms.dijkstra import MultiModalDijkstra
from src.simulation.demand_matrix import DemandMatrix, TimeWindow


class TransitProfiler:
    """
    Algorithmic Profiling Suite for Multi-Modal Public Transit System.
    Measures CPU execution time, function bottlenecks using cProfile,
    scaling behavior, and theoretical space complexity.
    """

    def __init__(self, graph: MultiModalGraph) -> None:
        self.graph = graph
        self.router = MultiModalDijkstra(graph)

    def profile_dijkstra_cprofile(self, query_count: int = 500) -> str:
        """
        Executes cProfile on a batch of multi-modal Dijkstra queries.
        Returns the formatted string of the top 15 time-consuming functions.
        """
        generator = DemandMatrix(self.graph)
        trips = generator.generate_trips(TimeWindow.MORNING_PEAK, total_passengers=query_count)

        pr = cProfile.Profile()
        pr.enable()

        # Profiling block
        for trip in trips:
            self.router.find_shortest_path(trip.origin_id, trip.destination_id)

        pr.disable()

        s = io.StringIO()
        ps = pstats.Stats(pr, stream=s).sort_stats(pstats.SortKey.CUMULATIVE)
        ps.print_stats(15)
        return s.getvalue()

    def benchmark_scaling(self, query_batches: List[int] = [50, 200, 500, 1000, 2500]) -> List[Dict[str, Any]]:
        """
        Measures CPU clock time across varying passenger query loads N.
        Evaluates algorithmic efficiency and throughput (queries / sec).
        """
        results: List[Dict[str, Any]] = []
        generator = DemandMatrix(self.graph)

        for n in query_batches:
            trips = generator.generate_trips(TimeWindow.MORNING_PEAK, total_passengers=n)

            start_time = time.perf_counter()
            for trip in trips:
                self.router.find_shortest_path(trip.origin_id, trip.destination_id)
            end_time = time.perf_counter()

            total_elapsed = end_time - start_time
            avg_per_query_ms = (total_elapsed / n) * 1000.0
            throughput = n / total_elapsed if total_elapsed > 0 else 0.0

            results.append({
                "queries_n": n,
                "total_time_sec": round(total_elapsed, 4),
                "avg_query_time_ms": round(avg_per_query_ms, 3),
                "throughput_qps": round(throughput, 1),
            })

        return results

    def get_space_complexity_metrics(self) -> Dict[str, Any]:
        """
        Calculates theoretical and actual memory footprints of the Adjacency List.
        """
        stats = self.graph.get_stats()
        v = stats["vertex_count"]
        e = stats["total_directed_edges"]

        return {
            "vertices_V": v,
            "edges_E": e,
            "theoretical_space": f"O(V + E) = O({v} + {e}) = {v + e} graph units",
            "adjacency_list_size_bytes": sys.getsizeof(self.graph.adjacency_list),
            "density": round((e) / (v * (v - 1)), 4) if v > 1 else 0.0,
        }


if __name__ == "__main__":
    import os
    base_dir = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    st_path = os.path.join(base_dir, "data", "stations.csv")
    rt_path = os.path.join(base_dir, "data", "routes.csv")

    graph = MultiModalGraph()
    graph.load_from_csv(st_path, rt_path)

    profiler = TransitProfiler(graph)
    print("=" * 60)
    print("RUNNING CPROFILE BENCHMARK")
    print("=" * 60)
    print(profiler.profile_dijkstra_cprofile(300))

    print("=" * 60)
    print("SCALING BENCHMARK")
    print("=" * 60)
    for res in profiler.benchmark_scaling([100, 500, 1000]):
        print(f"N={res['queries_n']} | Total: {res['total_time_sec']}s | "
              f"Per-Query: {res['avg_query_time_ms']} ms | QPS: {res['throughput_qps']}")
