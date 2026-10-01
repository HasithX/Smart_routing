from dataclasses import dataclass
from enum import Enum


class TransportMode(Enum):
    BUS = "BUS"
    TRAIN = "TRAIN"

    @classmethod
    def from_str(cls, label: str) -> "TransportMode":
        label_clean = label.strip().upper()
        if label_clean == "TRAIN":
            return cls.TRAIN
        return cls.BUS


@dataclass
class Edge:
    """
    Represents a directed edge E connecting two locations in the transit network.
    Contains mode type, base travel time, distance, and congestion tracking.
    """
    route_id: str
    source: str
    target: str
    mode: TransportMode
    distance_km: float
    base_time_min: float
    capacity: int = 100
    current_flow: int = 0

    def get_effective_travel_time(self) -> float:
        """
        Calculates real-time travel time incorporating traffic/passenger congestion.
        Applies a modified Bureau of Public Roads (BPR) impedance function:
        Time = BaseTime * (1 + alpha * (Flow / Capacity)^beta)
        
        Trains run on dedicated tracks, so their delay multiplier is much smaller.
        """
        if self.capacity <= 0:
            return self.base_time_min

        volume_to_capacity = self.current_flow / self.capacity

        if self.mode == TransportMode.TRAIN:
            # Train tracks are dedicated; minor dwell time delays at high capacity
            alpha = 0.15
            beta = 2.0
        else:
            # Buses on roads experience heavier congestion delays
            alpha = 0.60
            beta = 2.5

        congestion_multiplier = 1.0 + alpha * (volume_to_capacity ** beta)
        return round(self.base_time_min * congestion_multiplier, 2)

    def add_flow(self, passenger_count: int = 1) -> None:
        self.current_flow += passenger_count

    def reset_flow(self) -> None:
        self.current_flow = 0
