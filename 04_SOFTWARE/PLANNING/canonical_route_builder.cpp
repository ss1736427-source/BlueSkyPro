#include "canonical_route_builder.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <set>

namespace bluesky::planning {
namespace {

bool validPosition(const GeoPoint& p) {
    return std::isfinite(p.latitude_deg) && std::isfinite(p.longitude_deg)
        && p.latitude_deg >= -90.0 && p.latitude_deg <= 90.0
        && p.longitude_deg >= -180.0 && p.longitude_deg <= 180.0;
}

double distanceMeters(const GeoPoint& a, const GeoPoint& b) {
    constexpr double kEarthRadiusM = 6371008.8;
    constexpr double kPi = 3.14159265358979323846;
    const double lat1 = a.latitude_deg * kPi / 180.0;
    const double lat2 = b.latitude_deg * kPi / 180.0;
    const double dLat = lat2 - lat1;
    const double dLon = (b.longitude_deg - a.longitude_deg) * kPi / 180.0;
    const double h = std::sin(dLat / 2.0) * std::sin(dLat / 2.0)
        + std::cos(lat1) * std::cos(lat2)
        * std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
    return 2.0 * kEarthRadiusM * std::asin(std::sqrt(std::max(0.0, std::min(1.0, h))));
}

} // namespace

CanonicalRouteBuildResult CanonicalRouteBuilder::build(
    const SelectedRouteSet& selected,
    const PlanningGraph& graph,
    const std::string& route_id,
    const std::string& route_version,
    const std::string& generator_version) {
    CanonicalRouteBuildResult result;
    if (selected.mission_id.empty() || selected.mission_version.empty()
        || selected.candidate_id.empty() || route_id.empty() || route_version.empty()
        || generator_version.empty()) {
        result.error = "ROUTE_LINEAGE_REQUIRED";
        return result;
    }
    if (selected.route_elements.size() < 2) {
        result.error = "SELECTED_ROUTE_REQUIRES_AT_LEAST_TWO_ELEMENTS";
        return result;
    }
    if (selected.route_elements.front() != graph.start_node ||
        selected.route_elements.back() != graph.goal_node) {
        result.error = "SELECTED_ROUTE_ENDPOINTS_DO_NOT_MATCH_GRAPH";
        return result;
    }

    Route route;
    route.lineage.route_id = route_id;
    route.lineage.route_version = route_version;
    route.lineage.mission_id = selected.mission_id;
    route.lineage.mission_version = selected.mission_version;
    route.lineage.generator_id = "canonical_route_builder";
    route.lineage.generator_version = generator_version;
    route.lineage.calculation_input_version = selected.calculation_input_version;

    std::set<std::string> visited;
    for (const auto& elementId : selected.route_elements) {
        if (elementId.empty() || !visited.insert(elementId).second) {
            result.error = "EMPTY_OR_DUPLICATE_ROUTE_ELEMENT";
            return result;
        }
        const auto* node = graph.find_node(elementId);
        if (!node) {
            result.error = "SELECTED_ROUTE_ELEMENT_NOT_FOUND:" + elementId;
            return result;
        }
        if (!validPosition(node->position) || !std::isfinite(node->altitude_m)) {
            result.error = "INVALID_GEOGRAPHIC_ROUTE_NODE:" + elementId;
            return result;
        }
        route.waypoints.push_back({
            node->id, node->position, node->altitude_m,
            elementId == graph.start_node || elementId == graph.goal_node
        });
    }

    for (std::size_t i = 1; i < route.waypoints.size(); ++i) {
        const auto& from = route.waypoints[i - 1];
        const auto& to = route.waypoints[i];
        const bool connected = std::any_of(
            graph.edges.begin(), graph.edges.end(),
            [&](const PlanningGraphEdge& edge) {
                return edge.from == from.waypoint_id && edge.to == to.waypoint_id
                    && std::isfinite(edge.cost) && edge.cost >= 0.0;
            });
        if (!connected) {
            result.error = "SELECTED_ROUTE_EDGE_NOT_FOUND:" +
                from.waypoint_id + "->" + to.waypoint_id;
            return result;
        }
        route.segments.push_back({
            "SEG-" + std::to_string(i),
            from.waypoint_id,
            to.waypoint_id,
            distanceMeters(from.position, to.position),
            0.0
        });
    }

    result.valid = true;
    result.route = std::move(route);
    return result;
}

} // namespace bluesky::planning
