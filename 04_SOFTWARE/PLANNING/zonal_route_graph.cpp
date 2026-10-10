#include "zonal_route_graph.hpp"

#include <cmath>

namespace bluesky::planning {
namespace {

double cross(const GeoPoint& a, const GeoPoint& b, const GeoPoint& p) {
    return (b.longitude_deg - a.longitude_deg) * (p.latitude_deg - a.latitude_deg)
         - (b.latitude_deg - a.latitude_deg) * (p.longitude_deg - a.longitude_deg);
}

bool onSegment(const GeoPoint& a, const GeoPoint& b, const GeoPoint& p) {
    constexpr double eps = 1e-12;
    return std::abs(cross(a, b, p)) <= eps &&
           p.latitude_deg >= std::min(a.latitude_deg, b.latitude_deg) - eps &&
           p.latitude_deg <= std::max(a.latitude_deg, b.latitude_deg) + eps &&
           p.longitude_deg >= std::min(a.longitude_deg, b.longitude_deg) - eps &&
           p.longitude_deg <= std::max(a.longitude_deg, b.longitude_deg) + eps;
}

bool inside(const std::vector<GeoPoint>& polygon, const GeoPoint& p) {
    if (polygon.size() < 3) return false;
    bool hit = false;
    for (std::size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        const auto& a = polygon[j];
        const auto& b = polygon[i];
        if (onSegment(a, b, p)) return true;
        const bool crosses = ((a.longitude_deg > p.longitude_deg) !=
                              (b.longitude_deg > p.longitude_deg));
        if (crosses) {
            const double lat = a.latitude_deg +
                (b.latitude_deg - a.latitude_deg) *
                (p.longitude_deg - a.longitude_deg) /
                (b.longitude_deg - a.longitude_deg);
            if (p.latitude_deg < lat) hit = !hit;
        }
    }
    return hit;
}

} // namespace

ZonalRouteGraphResult ZonalRouteGraphBuilder::build(
    const PlanningGraph& source,
    const OperationalZone& zone) {
    ZonalRouteGraphResult result;
    result.zone_id = zone.zone_id;
    result.zone_version = zone.zone_version;

    for (const auto& node : source.nodes) {
        if (inside(zone.boundary, node.position)) {
            result.graph.nodes.push_back(node);
        } else {
            ++result.rejected_nodes;
        }
    }

    for (const auto& edge : source.edges) {
        const auto* from = source.find_node(edge.from);
        const auto* to = source.find_node(edge.to);
        if (!from || !to || !inside(zone.boundary, from->position) ||
            !inside(zone.boundary, to->position)) {
            ++result.rejected_edges;
            continue;
        }
        const GeoPoint mid{
            (from->position.latitude_deg + to->position.latitude_deg) * 0.5,
            (from->position.longitude_deg + to->position.longitude_deg) * 0.5
        };
        if (!inside(zone.boundary, mid)) {
            ++result.rejected_edges;
            continue;
        }
        result.graph.edges.push_back(edge);
    }

    result.graph.start_node = source.start_node;
    result.graph.goal_node = source.goal_node;
    if (!result.graph.find_node(result.graph.start_node)) result.graph.start_node.clear();
    if (!result.graph.find_node(result.graph.goal_node)) result.graph.goal_node.clear();

    result.dependency_identity =
        zone.zone_id + "|" + zone.zone_version + "|" + std::to_string(source.nodes.size()) +
        "|" + std::to_string(source.edges.size());

    return result;
}

} // namespace bluesky::planning
