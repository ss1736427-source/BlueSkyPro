#include "flight_profile.hpp"
#include <cmath>
namespace bluesky::planning {
FlightProfile FlightProfileBuilder::build(const SelectedRouteSet& selected,
                                          const Route& route,
                                          const TrajectoryResult& trajectory,
                                          const std::string& profile_id,
                                          const std::string& profile_version) {
    FlightProfile result;
    result.mission_id = selected.mission_id;
    result.mission_version = selected.mission_version;
    result.profile_id = profile_id;
    result.profile_version = profile_version;
    result.source_candidate_id = selected.candidate_id;
    result.route_version = route.lineage.route_version;
    result.calculation_input_version = route.lineage.calculation_input_version;
    result.dependency_identity = selected.source_dependency_identity + "|" + route.lineage.route_id + "|" + route.lineage.route_version + "|" + route.lineage.calculation_input_version;

    if (trajectory.status != TrajectoryStatus::Feasible) {
        result.validation_error = "Trajectory result is not feasible.";
        return result;
    }
    if (trajectory.route_id != route.lineage.route_id ||
        trajectory.route_version != route.lineage.route_version) {
        result.validation_error = "Trajectory route identity/version does not match the route.";
        return result;
    }
    if (route.waypoints.size() < 2 ||
        route.waypoints.size() != trajectory.points.size() ||
        route.segments.size() + 1 != route.waypoints.size() ||
        route.segments.size() != trajectory.segments.size()) {
        result.validation_error = "Route and trajectory point/segment counts do not match.";
        return result;
    }

    double segment_time_sum = 0.0;
    for (std::size_t i = 0; i < route.segments.size(); ++i) {
        const auto& route_segment = route.segments[i];
        const auto& trajectory_segment = trajectory.segments[i];
        if (route_segment.segment_id != trajectory_segment.segment_id ||
            route_segment.from_waypoint_id != route.waypoints[i].waypoint_id ||
            route_segment.to_waypoint_id != route.waypoints[i + 1].waypoint_id ||
            route_segment.from_waypoint_id != trajectory.points[i].waypoint_id ||
            route_segment.to_waypoint_id != trajectory.points[i + 1].waypoint_id ||
            !std::isfinite(trajectory_segment.traversal_time_s) ||
            trajectory_segment.traversal_time_s < 0.0 ||
            !std::isfinite(trajectory.points[i].elapsed_time_s) ||
            !std::isfinite(trajectory.points[i + 1].elapsed_time_s) ||
            std::abs((trajectory.points[i + 1].elapsed_time_s -
                      trajectory.points[i].elapsed_time_s) -
                     trajectory_segment.traversal_time_s) > 1e-6) {
            result.validation_error = "Trajectory segments do not match route order or contain invalid traversal times.";
            return result;
        }
        segment_time_sum += trajectory_segment.traversal_time_s;
    }
    if (!std::isfinite(segment_time_sum) ||
        std::abs(segment_time_sum - trajectory.total_time_s) > 1e-6) {
        result.validation_error = "Trajectory segment times do not sum to total trajectory time.";
        return result;
    }

    result.points.reserve(route.waypoints.size());
    double previous_time = -1.0;
    for (std::size_t i = 0; i < route.waypoints.size(); ++i) {
        const auto& wp = route.waypoints[i];
        const auto& tp = trajectory.points[i];
        if (wp.waypoint_id != tp.waypoint_id ||
            !std::isfinite(tp.elapsed_time_s) ||
            tp.elapsed_time_s < 0.0 ||
            tp.elapsed_time_s < previous_time ||
            !std::isfinite(wp.position.latitude_deg) ||
            !std::isfinite(wp.position.longitude_deg) ||
            !std::isfinite(wp.altitude_m) ||
            !std::isfinite(tp.position.latitude_deg) ||
            !std::isfinite(tp.position.longitude_deg) ||
            !std::isfinite(tp.altitude_m) ||
            std::abs(tp.position.latitude_deg - wp.position.latitude_deg) > 1e-8 ||
            std::abs(tp.position.longitude_deg - wp.position.longitude_deg) > 1e-8 ||
            std::abs(tp.altitude_m - wp.altitude_m) > 1e-6 ||
            (i == 0 && std::abs(tp.elapsed_time_s) > 1e-6)) {
            result.points.clear();
            result.validation_error = "Trajectory points do not match route order or contain invalid elapsed times.";
            return result;
        }
        FlightProfilePoint p;
        p.position = wp.position;
        p.altitude_m = wp.altitude_m;
        p.cumulative_time_s = tp.elapsed_time_s;
        p.mandatory = wp.mandatory;
        result.points.push_back(p);
        previous_time = tp.elapsed_time_s;
    }
    if (!std::isfinite(trajectory.total_time_s) || trajectory.total_time_s < 0.0 ||
        std::abs(result.points.back().cumulative_time_s - trajectory.total_time_s) > 1e-6) {
        result.points.clear();
        result.validation_error = "Final profile time does not match total trajectory time.";
        return result;
    }
    result.valid = true;
    return result;
}
}
