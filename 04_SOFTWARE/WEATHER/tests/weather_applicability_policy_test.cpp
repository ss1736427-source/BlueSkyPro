#include "weather_applicability_policy.hpp"

#include <cassert>
#include <iostream>

using namespace bluesky::weather;

int main()
{
    constexpr std::int64_t etd = 1'800'000'000;
    constexpr std::int64_t now = etd - 60;

    {
        const std::vector<AviationObservation> reports{
            {"AAA", "old", AviationObservationType::Metar, etd - 10801, etd - 10790},
            {"AAA", "metar", AviationObservationType::Metar, etd - 1800, etd - 1790},
            {"AAA", "speci", AviationObservationType::Speci, etd - 900, etd - 890},
            {"BBB", "other-station", AviationObservationType::Speci, etd - 100, etd - 90},
            {"AAA", "future", AviationObservationType::Metar, etd + 60, etd + 70}
        };
        const auto selected = WeatherApplicabilityPolicy::selectLatestObservation(
            reports, "AAA", etd, now);
        assert(selected && *selected == 1);
    }

    {
        const std::vector<AviationObservation> reports{
            {"AAA", "metar", AviationObservationType::Metar, etd - 900, etd - 890},
            {"AAA", "speci", AviationObservationType::Speci, etd - 900, etd - 880}
        };
        const auto selected = WeatherApplicabilityPolicy::selectLatestObservation(
            reports, "AAA", etd, now);
        assert(selected && *selected == 1);
    }

    {
        const std::vector<TafReport> reports{
            {"AAA", "taf-short", etd - 7200, etd - 7100, etd - 3600, etd + 3600, false},
            {"AAA", "taf-cover", etd - 3600, etd - 3500, etd - 1800, etd + 7200, false},
            {"AAA", "taf-cancelled", etd - 100, etd - 90, etd - 3600, etd + 7200, true}
        };
        const auto selected = WeatherApplicabilityPolicy::selectCoveringTaf(
            reports, "AAA", etd, etd + 5400, now);
        assert(selected && *selected == 1);
    }

    {
        const std::vector<TafReport> reports{
            {"AAA", "insufficient", etd - 3600, etd - 3500, etd - 1800, etd + 3600, false}
        };
        const auto selected = WeatherApplicabilityPolicy::selectCoveringTaf(
            reports, "AAA", etd, etd + 5400, now);
        assert(!selected);
    }

    assert(WeatherApplicabilityPolicy::departureAction(etd, etd + 900, false)
           == DepartureWeatherAction::ReuseSelectedWeather);
    assert(WeatherApplicabilityPolicy::departureAction(etd, etd + 901, false)
           == DepartureWeatherAction::ReevaluateWeather);
    assert(WeatherApplicabilityPolicy::departureAction(etd, etd + 60, true)
           == DepartureWeatherAction::ReevaluateWeather);

    std::cout << "Weather applicability policy tests passed\n";
    return 0;
}
