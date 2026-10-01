import os
import sys
from src.models.graph import MultiModalGraph
from src.algorithms.router import RoutePlanner
from src.simulation.demand_matrix import TimeWindow
from src.simulation.simulator import TransitSimulator
from src.profiling.profiler import TransitProfiler
from src.profiling.visualizer import TransitVisualizer


def print_banner():
    banner = """
========================================================================
   🏙️  SMART CITY PUBLIC TRANSIT ROUTING SYSTEM
   Advanced Data Structures & Algorithms | Multi-Modal Graph Engine
========================================================================
"""
    print(banner)


def initialize_network() -> MultiModalGraph:
    base_dir = os.path.dirname(os.path.abspath(__file__))
    stations_csv = os.path.join(base_dir, "data", "stations.csv")
    routes_csv = os.path.join(base_dir, "data", "routes.csv")

    graph = MultiModalGraph()
    graph.load_from_csv(stations_csv, routes_csv)
    return graph


def menu_view_stats(graph: MultiModalGraph):
    stats = graph.get_stats()
    print("\n--- Smart City Graph Topology & Stats ---")
    print(f"Total Stations / Stops (Vertices V) : {stats['vertex_count']}")
    print(f"Total Transit Routes (Edges E)      : {stats['total_directed_edges']}")
    print(f"  • Bus Directed Routes             : {stats['bus_directed_edges']}")
    print(f"  • Train Tracks                    : {stats['train_directed_edges']}")
    print(f"  • Multi-Modal Interchange Hubs    : {stats['interchange_hubs']}")
    print("\nStations Directory:")
    for nid, node in sorted(graph.nodes.items()):
        mode_tag = "🟡 HUB" if node.is_interchange else ("🔴 TRAIN" if node.node_type.value == "TRAIN_STATION" else "🔵 BUS")
        print(f"  [{nid}] {node.name:<26} | Type: {mode_tag:<10} | Zone: {node.zone}")


def menu_plan_route(planner: RoutePlanner, graph: MultiModalGraph):
    print("\n--- Plan Multi-Modal Journey ---")
    print("Available Station IDs: " + ", ".join(sorted(graph.nodes.keys())))
    orig = input("Enter Origin Station ID (e.g. ST01): ").strip().upper()
    dest = input("Enter Destination Station ID (e.g. ST06): ").strip().upper()

    if orig not in graph.nodes or dest not in graph.nodes:
        print("❌ Error: Invalid station ID provided.")
        return

    result = planner.plan_journey(orig, dest)
    print(planner.format_itinerary(result))


def menu_simulate_demand(graph: MultiModalGraph):
    print("\n--- Variable Passenger Demand Simulation ---")
    print("Select Time-of-Day Window:")
    print("1. Morning Peak  (07:00 - 09:30 AM) - Heavy suburban commute to commercial centers")
    print("2. Midday Off-Peak (11:00 - 02:00 PM) - Moderate, distributed travel")
    print("3. Evening Peak  (04:30 - 07:30 PM) - Heavy outbound commute back to suburbs")
    print("4. Night Low     (10:00 - 04:00 AM) - Sparse night traffic")

    choice = input("Enter option [1-4] (default 1): ").strip()
    window_map = {
        "1": TimeWindow.MORNING_PEAK,
        "2": TimeWindow.MIDDAY_OFF_PEAK,
        "3": TimeWindow.EVENING_PEAK,
        "4": TimeWindow.NIGHT_OFF_PEAK,
    }
    selected_window = window_map.get(choice, TimeWindow.MORNING_PEAK)
    passenger_input = input("Enter number of passengers to simulate (e.g. 300): ").strip()
    passenger_count = int(passenger_input) if passenger_input.isdigit() else 300

    print(f"\nSimulating {passenger_count} passenger journeys during {selected_window.value}...")
    sim = TransitSimulator(graph)
    results = sim.simulate_time_window(selected_window, passenger_count=passenger_count)

    print("\n" + "=" * 65)
    print(f" SIMULATION RESULTS: {results['time_window']}")
    print("=" * 65)
    print(f" Passengers Routed : {results['passengers_routed']} / {results['passengers_simulated']}")
    print(f" Average Trip Time : {results['avg_travel_time_min']} mins")
    print(f" Average Transfers : {results['avg_transfers_per_trip']} transfers per trip")
    print("\nTop 5 Congested Corridors / Bottlenecks:")
    print(f" {'Route ID':<10} {'Corridor':<20} {'Mode':<7} {'Flow/Cap':<12} {'Congestion Delay':<15}")
    print("-" * 65)
    for c in results["top_congested_edges"]:
        ratio_str = f"{c['flow']}/{c['capacity']} ({int(c['v_c_ratio']*100)}%)"
        delay_str = f"{c['base_time']}m -> {c['effective_time']}m"
        print(f" {c['route_id']:<10} {c['corridor']:<20} {c['mode']:<7} {ratio_str:<12} {delay_str:<15}")
    print("=" * 65)


def menu_profiling(graph: MultiModalGraph):
    profiler = TransitProfiler(graph)
    print("\n--- Algorithmic Profiling & Benchmarking ---")
    print("1. Run cProfile Bottleneck Analysis (300 queries)")
    print("2. Run Scaling & Throughput Test (N = 50, 200, 500, 1000, 2500)")
    print("3. Check Theoretical vs Practical Space Complexity")
    opt = input("Select profiling option [1-3]: ").strip()

    if opt == "1":
        print("\nExecuting cProfile on Multi-Modal Dijkstra...\n")
        report = profiler.profile_dijkstra_cprofile(300)
        print(report)
    elif opt == "2":
        print("\nMeasuring CPU execution time across query scaling loads...")
        benchmarks = profiler.benchmark_scaling([50, 200, 500, 1000, 2500])
        print("\n" + "=" * 65)
        print(f" {'Queries (N)':<12} {'Total Time (s)':<16} {'Avg/Query (ms)':<16} {'Throughput (QPS)':<15}")
        print("=" * 65)
        for b in benchmarks:
            print(f" {b['queries_n']:<12} {b['total_time_sec']:<16.4f} {b['avg_query_time_ms']:<16.3f} {b['throughput_qps']:<15.1f}")
        print("=" * 65)
    elif opt == "3":
        space = profiler.get_space_complexity_metrics()
        print("\n--- Space Complexity Analysis ---")
        print(f"Vertices (V)              : {space['vertices_V']}")
        print(f"Edges (E)                 : {space['edges_E']}")
        print(f"Adjacency List Complexity : {space['theoretical_space']}")
        print(f"Graph Density             : {space['density']}")
        print(f"Adjacency List Size       : {space['adjacency_list_size_bytes']} bytes in RAM")


def menu_visualize(graph: MultiModalGraph, planner: RoutePlanner):
    visualizer = TransitVisualizer(graph)
    print("\n--- Visualize Smart City Transit Network ---")
    print("1. Export Full City Transit Map (Bus & Train Network)")
    print("2. Export Transit Map with an Example Multi-Modal Route Highlighted")
    opt = input("Select option [1-2]: ").strip()

    if opt == "2":
        route = planner.plan_journey("ST01", "ST06")
        msg = visualizer.plot_transit_network(highlight_route=route, save_path="city_transit_route.png")
        print(f"✅ {msg}")
    else:
        msg = visualizer.plot_transit_network(save_path="city_transit_network.png")
        print(f"✅ {msg}")


def main():
    print_banner()
    graph = initialize_network()
    planner = RoutePlanner(graph)

    while True:
        print("\n" + "=" * 50)
        print(" MAIN MENU - SMART CITY TRANSIT SYSTEM")
        print("=" * 50)
        print(" 1. View Transit Network Topology & Graph Stats")
        print(" 2. Plan Multi-Modal Journey (Train + Bus + Transfer)")
        print(" 3. Simulate Variable Demand (Peak / Off-Peak)")
        print(" 4. Algorithmic Profiling (cProfile & CPU Benchmark)")
        print(" 5. Export Network Visual Map (NetworkX/Matplotlib)")
        print(" 6. Exit")
        choice = input("\nEnter choice [1-6]: ").strip()

        if choice == "1":
            menu_view_stats(graph)
        elif choice == "2":
            menu_plan_route(planner, graph)
        elif choice == "3":
            menu_simulate_demand(graph)
        elif choice == "4":
            menu_profiling(graph)
        elif choice == "5":
            menu_visualize(graph, planner)
        elif choice == "6":
            print("\nExiting Smart City Transit System. Good luck with the viva!\n")
            break
        else:
            print("Invalid choice, please select between 1 and 6.")


if __name__ == "__main__":
    main()
