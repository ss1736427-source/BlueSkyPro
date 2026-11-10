#include "weather_applicability_policy.hpp"

#include <algorithm>

namespace bluesky::weather {

std::optional<std::size_t> WeatherApplicabilityPolicy::selectLatestObservation(
    const std::vector<AviationObservation>& reports,
    const std::string& stationId,
    std::int64_t etdUtc,
    std::int64_t nowUtc)
{
    if (stationId.empty() || nowUtc > etdUtc)
        return std::nullopt;

    const std::int64_t earliest = etdUtc - kObservationWindowSeconds;
    std::optional<std::size_t> selected;
    for (std::size_t i = 0; i < reports.size(); ++i) {
        const auto& report = reports[i];
        if (report.station_id != stationId || report.report_id.empty()
            || report.observation_time_utc < earliest
            || report.observation_time_utc > etdUtc
            || report.observation_time_utc > nowUtc
            || report.received_time_utc > nowUtc
            || report.received_time_utc < report.observation_time_utc) {
            continue;
        }

        if (!selected) {
            selected = i;
            continue;
        }

        const auto& current = reports[*selected];
        if (report.observation_time_utc > current.observation_time_utc
            || (report.observation_time_utc == current.observation_time_utc
                && report.type == AviationObservationType::Speci
                && current.type != AviationObservationType::Speci)
            || (report.observation_time_utc == current.observation_time_utc
                && report.type == current.type
                && report.received_time_utc > current.received_time_utc)) {
            selected = i;
        }
    }
    return selected;
}

std::optional<std::size_t> WeatherApplicabilityPolicy::selectCoveringTaf(
    const std::vector<TafReport>& reports,
    const std::string& stationId,
    std::int64_t missionStartUtc,
    std::int64_t missionEndUtc,
    std::int64_t nowUtc)
{
    if (stationId.empty() || missionEndUtc < missionStartUtc)
        return std::nullopt;

    std::optional<std::size_t> selected;
    for (std::size_t i = 0; i < reports.size(); ++i) {
        const auto& report = reports[i];
        if (report.station_id != stationId || report.report_id.empty()
            || report.cancelled || report.issue_time_utc > nowUtc
            || report.received_time_utc > nowUtc
            || report.received_time_utc < report.issue_time_utc
            || report.valid_to_utc < report.valid_from_utc
            || report.valid_from_utc > missionStartUtc
            || report.valid_to_utc < missionEndUtc) {
            continue;
        }

        if (!selected || report.issue_time_utc > reports[*selected].issue_time_utc
            || (report.issue_time_utc == reports[*selected].issue_time_utc
                && report.received_time_utc > reports[*selected].received_time_utc)) {
            selected = i;
        }
    }
    return selected;
}

DepartureWeatherAction WeatherApplicabilityPolicy::departureAction(
    std::int64_t etdUtc,
    std::int64_t actualTakeoffUtc,
    bool speciReceived)
{
    if (speciReceived)
        return DepartureWeatherAction::ReevaluateWeather;

    const std::int64_t delay = actualTakeoffUtc > etdUtc
        ? actualTakeoffUtc - etdUtc : 0;
    return delay <= kOnTimeDepartureDelaySeconds
        ? DepartureWeatherAction::ReuseSelectedWeather
        : DepartureWeatherAction::ReevaluateWeather;
}

} // namespace bluesky::weather
