import random
from dataclasses import dataclass
from enum import Enum
from typing import List, Tuple
from src.models.graph import MultiModalGraph


class TimeWindow(Enum):
    MORNING_PEAK = "07:00 - 09:30 (Morning Peak Commute)"
    MIDDAY_OFF_PEAK = "11:00 - 14:00 (Midday Off-Peak)"
    EVENING_PEAK = "16:30 - 19:30 (Evening Peak Commute)"
    NIGHT_OFF_PEAK = "22:00 - 04:00 (Night Low Demand)"


@dataclass
class PassengerTrip:
    passenger_id: str
    origin_id: str
    destination_id: str
    departure_time: str
    time_window: TimeWindow


class DemandMatrix:
    """
    Simulates variable Origin-Destination (OD) passenger demand
    across different time-of-day windows.
    """

    def __init__(self, graph: MultiModalGraph, seed: int = 42) -> None:
        self.graph = graph
        self.random = random.Random(seed)

    def generate_trips(self, time_window: TimeWindow, total_passengers: int = 250) -> List[PassengerTrip]:
        """
        Generates simulated passenger trips based on realistic urban mobility patterns.
        """
        trips: List[PassengerTrip] = []

        all_node_ids = list(self.graph.nodes.keys())
        residential_nodes = [
            nid for nid, node in self.graph.nodes.items()
            if node.zone.lower() == "residential"
        ]
        commercial_nodes = [
            nid for nid, node in self.graph.nodes.items()
            if node.zone.lower() in ("commercial", "industrial", "educational")
        ]

        if not residential_nodes or not commercial_nodes:
            residential_nodes = all_node_ids
            commercial_nodes = all_node_ids

        for i in range(total_passengers):
            p_id = f"PX_{i+1:04d}"

            if time_window == TimeWindow.MORNING_PEAK:
                # 80% flow from suburbs/residential to commercial/work hubs
                if self.random.random() < 0.80:
                    orig = self.random.choice(residential_nodes)
                    dest = self.random.choice(commercial_nodes)
                else:
                    orig, dest = self.random.sample(all_node_ids, 2)
                dep_time = f"07:{self.random.randint(0, 59):02d} AM"

            elif time_window == TimeWindow.EVENING_PEAK:
                # 80% reverse flow from work hubs back to suburbs
                if self.random.random() < 0.80:
                    orig = self.random.choice(commercial_nodes)
                    dest = self.random.choice(residential_nodes)
                else:
                    orig, dest = self.random.sample(all_node_ids, 2)
                dep_time = f"05:{self.random.randint(0, 59):02d} PM"

            elif time_window == TimeWindow.MIDDAY_OFF_PEAK:
                # Uniform random travel across city
                orig, dest = self.random.sample(all_node_ids, 2)
                dep_time = f"01:{self.random.randint(0, 59):02d} PM"

            else:  # Night
                orig, dest = self.random.sample(all_node_ids, 2)
                dep_time = f"11:{self.random.randint(0, 59):02d} PM"

            # Ensure origin != destination
            while orig == dest:
                dest = self.random.choice(all_node_ids)

            trips.append(
                PassengerTrip(
                    passenger_id=p_id,
                    origin_id=orig,
                    destination_id=dest,
                    departure_time=dep_time,
                    time_window=time_window,
                )
            )

        return trips
