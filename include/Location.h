#ifndef LOCATION_H
#define LOCATION_H

#include <string>

using namespace std;

enum class NodeType {
    BUS_STOP,
    TRAIN_STATION,
    INTERCHANGE_HUB
};

string nodeTypeToString(NodeType type);
NodeType stringToNodeType(const string& str);

struct Location {
    string id;
    string name;
    NodeType nodeType;
    string zone;
    double x;
    double y;

    Location() : nodeType(NodeType::BUS_STOP), x(0.0), y(0.0) {}
    Location(string id, string name, NodeType type, string zone, double x, double y)
        : id(move(id)), name(move(name)), nodeType(type), zone(move(zone)), x(x), y(y) {}

    bool isInterchange() const {
        return nodeType == NodeType::INTERCHANGE_HUB;
    }
};

#endif // LOCATION_H
