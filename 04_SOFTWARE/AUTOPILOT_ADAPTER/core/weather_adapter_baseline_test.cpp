#include "weather_adapter_baseline.hpp"

#include <cassert>

using namespace bluesky::weather;

int main() {
    WeatherAdapterBaseline adapter;

    WeatherSample sample;
    sample.sourceId = "SRC-07";
    sample.sourceTimestampMs = 1700000000000;
    sample.windSpeedMps = 8.5;
    sample.windDirectionDeg = 245.0;
    sample.temperatureC = 18.0;
    sample.pressureHpa = 1013.2;
    sample.humidityPercent = 62.0;
    sample.state = WeatherState::Nominal;
    sample.quality = WeatherQuality::Valid;
    sample.valid = true;

    auto accepted = adapter.acceptSample(sample);
    assert(accepted.accepted);
    assert(adapter.readSample().has_value());
    assert(adapter.getState() == WeatherState::Nominal);
    assert(adapter.getQuality() == WeatherQuality::Valid);

    auto invalid = sample;
    invalid.humidityPercent = 101.0;
    auto invalidResult = adapter.acceptSample(invalid);
    assert(!invalidResult.accepted);
    assert(invalidResult.error == WeatherError::InvalidData);

    auto stale = sample;
    stale.stale = true;
    auto staleResult = adapter.acceptSample(stale);
    assert(!staleResult.accepted);
    assert(staleResult.error == WeatherError::StaleData);

    auto mismatch = sample;
    mismatch.sourceId = "SRC-OTHER";
    auto mismatchResult = adapter.acceptSample(mismatch);
    assert(!mismatchResult.accepted);
    assert(mismatchResult.error == WeatherError::ConfigurationMismatch);

    return 0;
}
