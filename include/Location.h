#ifndef LOCATION_H
#define LOCATION_H

#include <string>

enum class NodeType {
    BUS_STOP,
    TRAIN_STATION,
    INTERCHANGE_HUB
};

std::string nodeTypeToString(NodeType type);
NodeType stringToNodeType(const std::string& str);

struct Location {
    std::string id;
    std::string name;
    NodeType nodeType;
    std::string zone;
    double x;
    double y;

    Location() : nodeType(NodeType::BUS_STOP), x(0.0), y(0.0) {}
    Location(std::string id, std::string name, NodeType type, std::string zone, double x, double y)
        : id(std::move(id)), name(std::move(name)), nodeType(type), zone(std::move(zone)), x(x), y(y) {}

    bool isInterchange() const {
        return nodeType == NodeType::INTERCHANGE_HUB;
    }
};

#endif // LOCATION_H
