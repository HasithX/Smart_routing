#include "RoutePlanner.h"
#include <sstream>
#include <iomanip>

RouteResult RoutePlanner::planJourney(const std::string& originId, const std::string& destinationId) const {
    return router.findShortestPath(originId, destinationId);
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
    ss << std::string(68, '=') << "\n";
    ss << std::fixed << std::setprecision(1);
    ss << " Total Travel Time   : " << result.totalTimeMin << " mins\n";
    ss << " Total Distance      : " << result.totalDistanceKm << " km\n";
    ss << " Number of Transfers : " << result.transferCount << "\n";
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
