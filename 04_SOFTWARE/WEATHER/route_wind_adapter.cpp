#include "route_wind_adapter.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace bluesky::weather {
namespace {
void setError(std::string* out, const std::string& value)
{
    if (out) *out = value;
}

bool finitePoint(const bluesky::planning::GeoPoint& p)
{
    return std::isfinite(p.latitude_deg) && std::isfinite(p.longitude_deg) &&
           p.latitude_deg >= -90.0 && p.latitude_deg <= 90.0 &&
           p.longitude_deg >= -180.0 && p.longitude_deg <= 180.0;
}
}

bool RouteWindAdapter::apply(const bluesky::planning::Route& route,
                             const std::vector<WindField>& fields,
                             const std::string& snapshot_id,
                             const std::string& snapshot_version,
                             bluesky::planning::TrajectoryPerformanceInput& input,
                             std::string* error,
                             int maximum_vertical_gap_m)
{
    using namespace bluesky::planning;
    if (snapshot_id.empty() || snapshot_version.empty()) {
        setError(error, "wind snapshot identity and version are required");
        return false;
    }
    if (fields.empty() || route.waypoints.empty() ||
        route.segments.size() + 1 != route.waypoints.size()) {
        setError(error, "weather fields or route structure are incomplete");
        return false;
    }

    std::unordered_map<std::string, const RouteWaypoint*> waypoints;
    for (const auto& w : route.waypoints) {
        if (w.altitude_reference != AltitudeReference::AGL ||
            !std::isfinite(w.altitude_m) || w.altitude_m < 0.0 ||
            !finitePoint(w.position)) {
            setError(error, "every route waypoint must have finite coordinates and an explicit AGL altitude");
            return false;
        }
        if (!waypoints.emplace(w.waypoint_id, &w).second) {
            setError(error, "route contains duplicate waypoint identifiers");
            return false;
        }
    }

    const int validDate = fields.front().valid_date_utc;
    const int validTime = fields.front().valid_time_utc;
    for (const auto& f : fields) {
        if (f.valid_date_utc != validDate || f.valid_time_utc != validTime) {
            setError(error, "one route calculation cannot mix forecast valid times");
            return false;
        }
    }

    std::vector<WindSample> samples;
    samples.reserve(route.segments.size());
    for (const auto& segment : route.segments) {
        const auto fromIt = waypoints.find(segment.from_waypoint_id);
        const auto toIt = waypoints.find(segment.to_waypoint_id);
        if (fromIt == waypoints.end() || toIt == waypoints.end()) {
            setError(error, "route segment references an unknown waypoint: " + segment.segment_id);
            return false;
        }
        const auto& from = *fromIt->second;
        const auto& to = *toIt->second;
        const double latitude = (from.position.latitude_deg + to.position.latitude_deg) * 0.5;
        const double longitude = from.position.longitude_deg +
                                 (to.position.longitude_deg - from.position.longitude_deg) * 0.5;
        const double altitude = (from.altitude_m + to.altitude_m) * 0.5;
        const WindQuery query = WindInterpolator::atAltitude(
            fields, latitude, longitude, altitude, maximum_vertical_gap_m);
        if (!query.valid) {
            setError(error, "wind sample rejected for segment " + segment.segment_id +
                            ": " + query.detail);
            return false;
        }
        // GRIB U/V provides horizontal wind only. Vertical wind is not inferred.
        samples.push_back({segment.segment_id, query.northward_mps,
                           query.eastward_mps, 0.0});
    }

    input.wind_samples = std::move(samples);
    input.wind_snapshot_id = snapshot_id;
    input.wind_snapshot_version = snapshot_version;
    return true;
}

} // namespace bluesky::weather
