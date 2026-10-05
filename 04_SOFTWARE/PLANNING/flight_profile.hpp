#pragma once
#include "selected_route_set.hpp"
#include "model/route_model.hpp"
#include <string>
#include <vector>
namespace bluesky::planning {
struct FlightProfilePoint {
    GeoPoint position;
    double altitude_m{0.0};
    double cumulative_time_s{0.0};
    bool mandatory{false};
};
struct FlightProfile {
    std::string mission_id;
    std::string mission_version;
    std::string profile_id;
    std::string source_candidate_id;
    std::string route_version;
    std::string calculation_input_version;
    std::string dependency_identity;
    std::vector<FlightProfilePoint> points;
};
struct FlightProfileBuilder {
    static FlightProfile build(const SelectedRouteSet& selected,
                               const Route& route,
                               const std::string& profile_id,
                               const std::string& profile_version);
};
} // namespace bluesky::planning
