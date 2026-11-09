#pragma once

#include <string>
#include <vector>

namespace bluesky::weather {

enum class GfsForecastPlanStatus {
    Ready,
    InvalidRequest,
    InvalidCycleCandidate,
    NoAvailableCycle,
    NoSingleRunCoverage
};

struct GfsForecastCycleCandidate {
    // UTC cycle timestamp in YYYY-MM-DDTHH:00:00Z form.
    std::string cycle_time_utc;
    // True only after the acquisition layer has confirmed this cycle is usable.
    bool data_available{false};
};

struct GfsForecastPlanRequest {
    // Explicit mission execution window. The planner never derives this from a route.
    std::string mission_start_utc;
    std::string mission_end_utc;
    // Candidate cycles must come from a provider availability check or verified cache.
    std::vector<GfsForecastCycleCandidate> candidates;
};

struct GfsForecastPlan {
    GfsForecastPlanStatus status{GfsForecastPlanStatus::InvalidRequest};
    std::string selected_cycle_utc;
    // Ordered GFS forecast lead hours required to bracket the full mission window.
    std::vector<int> forecast_lead_hours;
    std::string reason;

    bool ready() const noexcept { return status == GfsForecastPlanStatus::Ready; }
};

// Plans one GFS 0.25-degree forecast run for a fully specified UTC mission window.
// GFS lead cadence: hourly for F000-F120, then every three hours through F384.
// The planner selects only explicitly confirmed available cycles and does not
// infer data availability, mission timing, or temporal interpolation policy.
class GfsForecastPlanner final {
public:
    static GfsForecastPlan build(const GfsForecastPlanRequest& request);
};

} // namespace bluesky::weather
