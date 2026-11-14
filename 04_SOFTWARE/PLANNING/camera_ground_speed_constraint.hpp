#pragma once

#include "wind_performance_trajectory.hpp"
#include "variable_speed_profile.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace bluesky::planning {

struct CameraAirspeedCapResult {
    bool feasible{false};
    std::string segment_id;
    double wind_along_track_mps{0.0};
    double wind_cross_track_mps{0.0};
    double camera_maximum_ground_speed_mps{0.0};
    double minimum_airspeed_mps{0.0};
    double maximum_airspeed_mps{0.0};
    std::string diagnostic;
};

class CameraGroundSpeedConstraint {
public:
    static CameraAirspeedCapResult deriveAirspeedCap(
        double track_deg,
        const WindSample& wind,
        double camera_maximum_ground_speed_mps,
        double aircraft_minimum_airspeed_mps,
        double aircraft_maximum_airspeed_mps)
    {
        CameraAirspeedCapResult out;
        out.segment_id = wind.segment_id;
        out.camera_maximum_ground_speed_mps = camera_maximum_ground_speed_mps;
        if (!std::isfinite(track_deg) || !std::isfinite(wind.north_mps)
            || !std::isfinite(wind.east_mps)
            || !std::isfinite(camera_maximum_ground_speed_mps)
            || camera_maximum_ground_speed_mps <= 0.0
            || !std::isfinite(aircraft_minimum_airspeed_mps)
            || aircraft_minimum_airspeed_mps <= 0.0
            || !std::isfinite(aircraft_maximum_airspeed_mps)
            || aircraft_maximum_airspeed_mps < aircraft_minimum_airspeed_mps) {
            out.diagnostic = "CAMERA_AIRSPEED_CAP_INPUT_INVALID";
            return out;
        }

        constexpr double kPi = 3.14159265358979323846;
        const double track = track_deg * kPi / 180.0;
        const double hn = std::cos(track);
        const double he = std::sin(track);
        out.wind_along_track_mps = wind.north_mps * hn + wind.east_mps * he;
        out.wind_cross_track_mps = -wind.north_mps * he + wind.east_mps * hn;

        // Vg = sqrt(Va^2 - Wcross^2) + Walong for a crab-corrected track.
        const double availableAlongGroundSpeed =
            camera_maximum_ground_speed_mps - out.wind_along_track_mps;
        if (availableAlongGroundSpeed < -1e-9
            || (availableAlongGroundSpeed <= 1e-9 && out.wind_along_track_mps <= 1e-9)) {
            out.diagnostic = "CAMERA_GROUND_SPEED_CAP_UNACHIEVABLE_UNDER_TAILWIND";
            return out;
        }
        const double capSquared = availableAlongGroundSpeed * availableAlongGroundSpeed
            + out.wind_cross_track_mps * out.wind_cross_track_mps;
        const double windAwareCap = std::sqrt(capSquared);
        // The profile lower bound must also guarantee crosswind compensation
        // and positive progress against a headwind.
        const double minimumForWind = std::hypot(
            out.wind_cross_track_mps,
            std::max(0.0, -out.wind_along_track_mps));
        // A positive tailwind can still provide forward progress at exactly
        // the crosswind-compensation airspeed; zero/headwind needs a margin.
        const double progressMargin = out.wind_along_track_mps > 1e-9 ? 0.0 : 1e-6;
        out.minimum_airspeed_mps = std::max(
            aircraft_minimum_airspeed_mps, minimumForWind + progressMargin);
        out.maximum_airspeed_mps = std::min(aircraft_maximum_airspeed_mps, windAwareCap);
        if (out.maximum_airspeed_mps + 1e-9 < out.minimum_airspeed_mps) {
            out.diagnostic = "CAMERA_GROUND_SPEED_CAP_CONFLICTS_WITH_AIRCRAFT_OR_WIND_MINIMUM_AIRSPEED";
            return out;
        }
        out.feasible = true;
        out.diagnostic = "CAMERA_AIRSPEED_CAP_DERIVED_WITH_CRAB_CORRECTION";
        return out;
    }

    // Apply the wind-aware camera envelope directly to one solver segment.
    // Segment identity is checked so caps cannot be attached to another leg.
    static bool applyToSegment(
        const CameraAirspeedCapResult& cap,
        SpeedProfileSegmentInput& segment)
    {
        if (!cap.feasible || cap.segment_id.empty()
            || cap.segment_id != segment.segment_id
            || !std::isfinite(segment.minimum_speed_mps)
            || !std::isfinite(segment.maximum_speed_mps)
            || segment.minimum_speed_mps <= 0.0
            || segment.maximum_speed_mps < segment.minimum_speed_mps)
            return false;
        segment.minimum_speed_mps = std::max(
            segment.minimum_speed_mps, cap.minimum_airspeed_mps);
        segment.maximum_speed_mps = std::min(
            segment.maximum_speed_mps, cap.maximum_airspeed_mps);
        return segment.minimum_speed_mps <= segment.maximum_speed_mps + 1e-9;
    }
};

} // namespace bluesky::planning
