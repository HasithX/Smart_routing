#include "Location.h"
#include <algorithm>

std::string nodeTypeToString(NodeType type) {
    switch (type) {
        case NodeType::TRAIN_STATION: return "TRAIN_STATION";
        case NodeType::INTERCHANGE_HUB: return "INTERCHANGE_HUB";
        case NodeType::BUS_STOP:
        default: return "BUS_STOP";
    }
}

NodeType stringToNodeType(const std::string& str) {
    std::string s = str;
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    if (s == "TRAIN_STATION" || s == "TRAIN") return NodeType::TRAIN_STATION;
    if (s == "INTERCHANGE_HUB" || s == "HUB") return NodeType::INTERCHANGE_HUB;
    return NodeType::BUS_STOP;
}
