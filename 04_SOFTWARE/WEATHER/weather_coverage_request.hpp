#pragma once

#include "../PLANNING/model/mission_model.hpp"
#include "../PLANNING/model/route_model.hpp"

#include <string>
#include <vector>

namespace bluesky::weather {

enum class CoverageComponentKind {
    AreaOfOperations,
    Route,
    PlannedOrigin
};

struct CoverageCoordinate {
    double latitude_deg{0.0};
    double longitude_deg{0.0};
};

struct CoverageComponent {
    CoverageComponentKind kind{CoverageComponentKind::AreaOfOperations};
    std::string source_id;
    std::string source_version;
    std::string geometry_type;
    std::vector<CoverageCoordinate> coordinates;
};

struct GeographicCoverageBounds {
    double south_deg{0.0};
    double west_deg{0.0};
    double north_deg{0.0};
    double east_deg{0.0};
};

struct WeatherCoverageWindow {
    // Strict UTC timestamps: YYYY-MM-DDTHH:MM:SSZ.
    std::string valid_from_utc;
    std::string valid_to_utc;
};

struct WeatherCoverageRequest {
    std::string request_schema_version{"1"};
    std::string mission_id;
    std::string mission_version;
    std::string route_id;
    std::string route_version;
    std::string calculation_input_version;
    std::vector<CoverageComponent> components;
    GeographicCoverageBounds bounds;
    double spatial_buffer_m{0.0};
    WeatherCoverageWindow time_window;
    std::vector<double> required_altitudes_agl_m;
};

struct WeatherCoverageRequestOptions {
    double spatial_buffer_m{0.0};
    WeatherCoverageWindow time_window;
    // Explicit AGL levels required for area/task coverage. Route waypoint
    // altitudes are added automatically after their AGL datum is validated.
    std::vector<double> required_area_altitudes_agl_m;
};

struct WeatherCoverageRequestBuildResult {
    bool valid{false};
    WeatherCoverageRequest request;
    std::vector<std::string> errors;
};

class WeatherCoverageRequestBuilder final {
public:
    // Builds a mission-specific spatial-temporal request from the existing
    // canonical Mission and selected Route. This creates a conservative
    // provider bounding box; it does not claim polygon-cell coverage or
    // validate forecast data.
    static WeatherCoverageRequestBuildResult build(
        const bluesky::planning::Mission& mission,
        const bluesky::planning::Route& route,
        const WeatherCoverageRequestOptions& options);
};

} // namespace bluesky::weather
