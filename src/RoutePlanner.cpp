#include "RoutePlanner.h"
#include <sstream>
#include <iomanip>

RouteResult RoutePlanner::planJourney(const std::string& originId,
                                      const std::string& destinationId,
                                      RoutingPreference preference) const {
    return router.findOptimalPath(originId, destinationId, preference);
}

std::string RoutePlanner::formatItinerary(const RouteResult& result) const {
    if (!result.isReachable()) {
        return "No public transit route found between " + result.origin + " and " + result.destination + ".\n";
    }

    const Location* origNode = graph.getNode(result.origin);
    const Location* destNode = graph.getNode(result.destination);
    std::string origName = origNode ? origNode->name : result.origin;
    std::string destName = destNode ? destNode->name : result.destination;

    std::stringstream ss;
    ss << std::string(68, '=') << "\n";
    ss << " SMART CITY TRANSIT ITINERARY: " << origName << " -> " << destName << "\n";
    ss << " Routing Criteria   : " << routingPreferenceToString(result.preference) << "\n";
    ss << std::string(68, '=') << "\n";
    ss << std::fixed << std::setprecision(1);
    ss << " Total Travel Time   : " << result.totalTimeMin << " mins (incl. transfers)\n";
    ss << " Total Distance      : " << result.totalDistanceKm << " km\n";
    ss << " Number of Transfers : " << result.transferCount << " transfer(s)\n";
    ss << std::string(68, '-') << "\n";
    ss << " Step-by-step transit directions:\n";

    int stepNum = 1;
    for (const auto& leg : result.legs) {
        const Location* srcNode = graph.getNode(leg.source);
        const Location* tgtNode = graph.getNode(leg.target);
        std::string srcName = srcNode ? srcNode->name : leg.source;
        std::string tgtName = tgtNode ? tgtNode->name : leg.target;

        if (leg.isTransferBefore) {
            ss << "   [" << stepNum++ << "] TRANSFER: Change platforms / Walk at '"
               << srcName << "' (+" << leg.transferPenaltyMin << " min penalty)\n";
        }

        std::string modeSymbol = (leg.mode == TransportMode::TRAIN) ? "[Train]" : "[Bus]";
        ss << "   [" << stepNum++ << "] " << modeSymbol << " from '" << srcName << "' to '"
           << tgtName << "' (" << leg.distanceKm << " km | " << leg.travelTimeMin << " mins)\n";
    }

    ss << std::string(68, '=') << "\n";
    return ss.str();
}

std::string RoutePlanner::comparePreferences(const std::string& originId, const std::string& destinationId) const {
    RouteResult fast = planJourney(originId, destinationId, RoutingPreference::FASTEST_TIME);
    RouteResult direct = planJourney(originId, destinationId, RoutingPreference::MINIMUM_TRANSFERS);

    const Location* origNode = graph.getNode(originId);
    const Location* destNode = graph.getNode(destinationId);
    std::string origName = origNode ? origNode->name : originId;
    std::string destName = destNode ? destNode->name : destinationId;

    std::stringstream ss;
    ss << "\n" << std::string(72, '=') << "\n";
    ss << " PARETO MULTI-CRITERIA ROUTING COMPARISON (Hasitha's Algorithmic Engine)\n";
    ss << " Journey: " << origName << " -> " << destName << "\n";
    ss << std::string(72, '=') << "\n";
    ss << std::fixed << std::setprecision(1);

    ss << " [Criteria 1: Fastest Travel Time]\n";
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

    ss << "\n [Criteria 2: Minimum Transfers (Elderly / Luggage Friendly)]\n";
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

    ss << std::string(72, '-') << "\n";
    if (fast.isReachable() && direct.isReachable()) {
        if (fast.transferCount != direct.transferCount) {
            double timeDiff = direct.totalTimeMin - fast.totalTimeMin;
            ss << " Pareto Trade-off Insight: Minimum transfers route saves "
               << (fast.transferCount - direct.transferCount) << " transfer(s) at the cost of "
               << timeDiff << " additional minutes.\n";
        } else {
            ss << " Both criteria converge on the same globally optimal path for this OD pair.\n";
        }
    }
    ss << std::string(72, '=') << "\n";
    return ss.str();
}
