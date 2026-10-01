from dataclasses import dataclass
from enum import Enum
from typing import Optional


class NodeType(Enum):
    BUS_STOP = "BUS_STOP"
    TRAIN_STATION = "TRAIN_STATION"
    INTERCHANGE_HUB = "INTERCHANGE_HUB"

    @classmethod
    def from_str(cls, label: str) -> "NodeType":
        label_clean = label.strip().upper()
        for item in cls:
            if item.value == label_clean:
                return item
        return cls.BUS_STOP


@dataclass
class Location:
    """
    Represents a vertex V in the multi-modal public transit network.
    Could be a pure bus stop, train station, or multi-modal interchange hub.
    """
    id: str
    name: str
    node_type: NodeType
    zone: str
    x: float = 0.0
    y: float = 0.0

    @property
    def is_interchange(self) -> bool:
        """Returns True if passengers can switch between Bus and Train here."""
        return self.node_type == NodeType.INTERCHANGE_HUB

    def __repr__(self) -> str:
        return f"Location({self.id}, {self.name}, {self.node_type.value})"
