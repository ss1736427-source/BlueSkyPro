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
    if (route.waypoints.size() != trajectory.points.size() ||
        route.segments.size() + 1 != route.waypoints.size()) {
        result.validation_error = "Route and trajectory point/segment counts do not match.";
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
            tp.elapsed_time_s < previous_time) {
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
    if (!std::isfinite(trajectory.total_time_s) ||
        std::abs(result.points.back().cumulative_time_s - trajectory.total_time_s) > 1e-6) {
        result.points.clear();
        result.validation_error = "Final profile time does not match total trajectory time.";
        return result;
    }
    result.valid = true;
    return result;
}
}
