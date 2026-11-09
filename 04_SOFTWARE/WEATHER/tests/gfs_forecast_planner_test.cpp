#include "gfs_forecast_planner.hpp"

#include <cassert>
#include <iostream>

using namespace bluesky::weather;

int main()
{
    {
        GfsForecastPlanRequest request;
        request.mission_start_utc = "2026-11-09T09:30:00Z";
        request.mission_end_utc = "2026-11-09T12:30:00Z";
        request.candidates = {
            {"2026-11-09T00:00:00Z", true},
            {"2026-11-09T06:00:00Z", true}
        };
        const auto plan = GfsForecastPlanner::build(request);
        assert(plan.ready());
        assert(plan.selected_cycle_utc == "2026-11-09T06:00:00Z");
        assert((plan.forecast_lead_hours == std::vector<int>{3, 4, 5, 6, 7}));
    }

    {
        GfsForecastPlanRequest request;
        request.mission_start_utc = "2026-11-14T05:30:00Z";
        request.mission_end_utc = "2026-11-14T06:30:00Z";
        request.candidates = {{"2026-11-09T00:00:00Z", true}};
        const auto plan = GfsForecastPlanner::build(request);
        assert(plan.ready());
        assert((plan.forecast_lead_hours == std::vector<int>{123, 126, 129}));
    }

    {
        GfsForecastPlanRequest request;
        request.mission_start_utc = "2026-11-09T06:00:00Z";
        request.mission_end_utc = "2026-11-09T06:00:00Z";
        request.candidates = {{"2026-11-09T06:00:00Z", true}};
        const auto plan = GfsForecastPlanner::build(request);
        assert(plan.ready());
        assert((plan.forecast_lead_hours == std::vector<int>{0}));
    }

    {
        GfsForecastPlanRequest request;
        request.mission_start_utc = "2026-11-09T06:00:00Z";
        request.mission_end_utc = "2026-11-09T07:00:00Z";
        request.candidates = {{"2026-11-09T06:00:00Z", false}};
        const auto plan = GfsForecastPlanner::build(request);
        assert(plan.status == GfsForecastPlanStatus::NoAvailableCycle);
    }

    {
        GfsForecastPlanRequest request;
        request.mission_start_utc = "2026-11-09T06:00:00Z";
        request.mission_end_utc = "2026-11-09T07:00:00Z";
        request.candidates = {{"2026-11-09T03:00:00Z", true}};
        const auto plan = GfsForecastPlanner::build(request);
        assert(plan.status == GfsForecastPlanStatus::InvalidCycleCandidate);
    }

    {
        GfsForecastPlanRequest request;
        request.mission_start_utc = "2026-02-29T06:00:00Z";
        request.mission_end_utc = "2026-03-01T07:00:00Z";
        request.candidates = {{"2026-02-28T06:00:00Z", true}};
        const auto plan = GfsForecastPlanner::build(request);
        assert(plan.status == GfsForecastPlanStatus::InvalidRequest);
    }

    {
        GfsForecastPlanRequest request;
        request.mission_start_utc = "2026-11-26T00:00:00Z";
        request.mission_end_utc = "2026-11-26T01:00:00Z";
        request.candidates = {{"2026-11-09T00:00:00Z", true}};
        const auto plan = GfsForecastPlanner::build(request);
        assert(plan.status == GfsForecastPlanStatus::NoSingleRunCoverage);
    }

    std::cout << "GFS forecast planner contract tests passed\n";
    return 0;
}
