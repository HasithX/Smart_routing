#include "RoutePlanner.h"
#include <sstream>
#include <iomanip>

using namespace std;

RouteResult RoutePlanner::planJourney(const string& originId,
                                      const string& destinationId,
                                      RoutingPreference preference) const {
    return router.findOptimalPath(originId, destinationId, preference);
}

string RoutePlanner::formatItinerary(const RouteResult& result) const {
    if (!result.isReachable()) {
        return "No public transit route found between " + result.origin + " and " + result.destination + ".\n";
    }

    const Location* origNode = graph.getNode(result.origin);
    const Location* destNode = graph.getNode(result.destination);
    string origName = origNode ? origNode->name : result.origin;
    string destName = destNode ? destNode->name : result.destination;

    stringstream ss;
    ss << string(68, '=') << "\n";
    ss << " SMART CITY TRANSIT ITINERARY: " << origName << " -> " << destName << "\n";
    ss << " Routing Criteria   : " << routingPreferenceToString(result.preference) << "\n";
    ss << string(68, '=') << "\n";
    ss << fixed << setprecision(1);
    ss << " Total Travel Time   : " << result.totalTimeMin << " mins (incl. transfers)\n";
    ss << " Total Distance      : " << result.totalDistanceKm << " km\n";
    ss << " Number of Transfers : " << result.transferCount << " transfer(s)\n";
    ss << string(68, '-') << "\n";
    ss << " Step-by-step transit directions:\n";

    int stepNum = 1;
    for (const auto& leg : result.legs) {
        const Location* srcNode = graph.getNode(leg.source);
        const Location* tgtNode = graph.getNode(leg.target);
        string srcName = srcNode ? srcNode->name : leg.source;
        string tgtName = tgtNode ? tgtNode->name : leg.target;

        if (leg.isTransferBefore) {
            ss << "   [" << stepNum++ << "] TRANSFER: Change platforms / Walk at '"
               << srcName << "' (+" << leg.transferPenaltyMin << " min penalty)\n";
        }

        string modeSymbol = (leg.mode == TransportMode::TRAIN) ? "[Train]" : "[Bus]";
        ss << "   [" << stepNum++ << "] " << modeSymbol << " from '" << srcName << "' to '"
           << tgtName << "' (" << leg.distanceKm << " km | " << leg.travelTimeMin << " mins)\n";
    }

    ss << string(68, '=') << "\n";
    return ss.str();
}

string RoutePlanner::comparePreferences(const string& originId, const string& destinationId) const {
    RouteResult fast = planJourney(originId, destinationId, RoutingPreference::FASTEST_TIME);
    RouteResult direct = planJourney(originId, destinationId, RoutingPreference::MINIMUM_TRANSFERS);

    const Location* origNode = graph.getNode(originId);
    const Location* destNode = graph.getNode(destinationId);
    string origName = origNode ? origNode->name : originId;
    string destName = destNode ? destNode->name : destinationId;

    stringstream ss;
    ss << "\n" << string(68, '=') << "\n";
    ss << " ROUTING COMPARISON: " << origName << " -> " << destName << "\n";
    ss << string(68, '=') << "\n";
    ss << fixed << setprecision(1);

    ss << " [1. Fastest Travel Time]\n";
    if (fast.isReachable()) {
        ss << "   • Travel Time : " << fast.totalTimeMin << " mins\n";
        ss << "   • Distance    : " << fast.totalDistanceKm << " km\n";
        ss << "   • Transfers   : " << fast.transferCount << " transfer(s)\n";
        ss << "   • Path        : ";
        for (size_t i = 0; i < fast.pathNodes.size(); ++i) {
            ss << fast.pathNodes[i] << (i + 1 < fast.pathNodes.size() ? " -> " : "\n");
        }
    } else {
        ss << "   • Unreachable\n";
    }

    ss << "\n [2. Minimum Transfers (Direct Route Priority)]\n";
    if (direct.isReachable()) {
        ss << "   • Travel Time : " << direct.totalTimeMin << " mins\n";
        ss << "   • Distance    : " << direct.totalDistanceKm << " km\n";
        ss << "   • Transfers   : " << direct.transferCount << " transfer(s)\n";
        ss << "   • Path        : ";
        for (size_t i = 0; i < direct.pathNodes.size(); ++i) {
            ss << direct.pathNodes[i] << (i + 1 < direct.pathNodes.size() ? " -> " : "\n");
        }
    } else {
        ss << "   • Unreachable\n";
    }

    ss << string(68, '-') << "\n";
    if (fast.isReachable() && direct.isReachable()) {
        if (fast.transferCount != direct.transferCount) {
            double timeDiff = direct.totalTimeMin - fast.totalTimeMin;
            ss << " Trade-off: Minimum transfers saves "
               << (fast.transferCount - direct.transferCount) << " transfer(s) (+ "
               << timeDiff << " mins longer).\n";
        } else {
            ss << " Both criteria use the same optimal route.\n";
        }
    }
    ss << string(68, '=') << "\n";
    return ss.str();
}
