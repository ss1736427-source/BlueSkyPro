#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bluesky::weather {

// All timestamps are Unix seconds on the UTC timeline. The caller must parse
// and validate source timestamps before constructing these records.
enum class AviationObservationType { Metar, Speci };

struct AviationObservation {
    std::string station_id;
    std::string report_id;
    AviationObservationType type{AviationObservationType::Metar};
    std::int64_t observation_time_utc{0};
    std::int64_t received_time_utc{0};
};

struct TafReport {
    std::string station_id;
    std::string report_id;
    std::int64_t issue_time_utc{0};
    std::int64_t received_time_utc{0};
    std::int64_t valid_from_utc{0};
    std::int64_t valid_to_utc{0};
    bool cancelled{false};
};

enum class DepartureWeatherAction {
    ReuseSelectedWeather,
    ReevaluateWeather
};

class WeatherApplicabilityPolicy final {
public:
    static constexpr std::int64_t kObservationWindowSeconds = 3 * 60 * 60;
    static constexpr std::int64_t kOnTimeDepartureDelaySeconds = 15 * 60;

    // Selects the newest observation for the requested station in the
    // inclusive ETD +/- 3h planning window, excluding future/unreceived data.
    // The observation window is inclusive on both sides of ETD. Only reports
    // already observed and received by nowUtc can be selected.
    static std::optional<std::size_t> selectLatestObservation(
        const std::vector<AviationObservation>& reports,
        const std::string& stationId,
        std::int64_t etdUtc,
        std::int64_t nowUtc);

    // TAF validity is taken from the encoded validity interval. No generic
    // three-hour extension is applied. Newer issued TAF wins; cancelled
    // products are never selected.
    static std::optional<std::size_t> selectCoveringTaf(
        const std::vector<TafReport>& reports,
        const std::string& stationId,
        std::int64_t missionStartUtc,
        std::int64_t missionEndUtc,
        std::int64_t nowUtc);

    // A SPECI event always causes an immediate weather re-evaluation.
    // A departure delay <= 15 min alone does not. A later delay does.
    static DepartureWeatherAction departureAction(
        std::int64_t etdUtc,
        std::int64_t actualTakeoffUtc,
        bool speciReceived);
};

} // namespace bluesky::weather
