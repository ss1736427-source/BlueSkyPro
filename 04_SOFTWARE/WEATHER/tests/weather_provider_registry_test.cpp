#include "weather_provider_registry.hpp"

#include <cassert>
#include <memory>

using namespace bluesky::weather;

namespace {
class TestGfsAdapter final : public IWeatherProviderAdapter {
public:
    WeatherProviderDescriptor descriptor() const override
    {
        WeatherProviderCapability wind;
        wind.product = WeatherProductType::NumericalWindField;
        wind.coverage = {90.0, -10.0, 160.0, 30.0};
        wind.minimumAltitudeAglM = 10.0;
        wind.maximumAltitudeAglM = 1000.0;
        wind.altitudeLevelsAglM = {10.0, 20.0, 30.0, 40.0, 50.0, 80.0, 100.0, 1000.0};
        wind.maximumVerticalInterpolationGapM = 100;
        wind.maximumForecastHorizonSeconds = 384LL * 3600LL;
        wind.minimumGridSpacingDegrees = 0.25;
        return {"noaa.gfs.025", "NOAA GFS 0.25 degree", 1, {wind}};
    }
};
}

int main()
{
    WeatherProviderRegistry registry;
    std::string error;
    assert(registry.registerAdapter(std::make_shared<TestGfsAdapter>(), &error));
    assert(!registry.registerAdapter(std::make_shared<TestGfsAdapter>(), &error));
    assert(registry.upsertInstance(
        {"noaa-gfs-primary", "noaa.gfs.025", "NOAA GFS primary", true, 10}, &error));
    assert(registry.upsertInstance(
        {"noaa-gfs-standby", "noaa.gfs.025", "NOAA GFS standby", false, 20}, &error));

    WeatherProviderRequest request;
    request.product = WeatherProductType::NumericalWindField;
    request.area = {100.0, 0.0, 120.0, 20.0};
    request.minimumAltitudeAglM = 10.0;
    request.maximumAltitudeAglM = 100.0;
    request.referenceTimeUtcSeconds = 1'800'000'000;
    request.validFromUtcSeconds = request.referenceTimeUtcSeconds;
    request.validToUtcSeconds = request.referenceTimeUtcSeconds + 48LL * 3600LL;
    request.maximumGridSpacingDegrees = 0.5;
    request.requiredAltitudesAglM = {50.0, 80.0};

    auto selected = registry.selectEligible(request);
    assert(selected.size() == 1);
    assert(selected.front().instance.instanceId == "noaa-gfs-primary");

    request.area.east = 170.0;
    assert(registry.selectEligible(request).empty());

    request.area = {100.0, 0.0, 120.0, 20.0};
    request.maximumAltitudeAglM = 1200.0;
    assert(registry.selectEligible(request).empty());

    request.maximumAltitudeAglM = 1000.0;
    request.requiredAltitudesAglM = {200.0};
    assert(registry.selectEligible(request).empty()); // 100-1000 m gap exceeds the 100 m limit
    request.requiredAltitudesAglM = {65.0};
    selected = registry.selectEligible(request);
    assert(selected.size() == 1); // bounded interpolation between 50 and 80 m
    request.maximumAltitudeAglM = 100.0;
    request.requiredAltitudesAglM = {50.0, 80.0};
    request.validToUtcSeconds = request.referenceTimeUtcSeconds + 385LL * 3600LL;
    assert(registry.selectEligible(request).empty());

    request.validToUtcSeconds = request.referenceTimeUtcSeconds + 48LL * 3600LL;
    assert(registry.setEnabled("noaa-gfs-standby", true));
    selected = registry.selectEligible(request);
    assert(selected.size() == 2);
    assert(selected[0].instance.instanceId == "noaa-gfs-primary");
    assert(selected[1].instance.instanceId == "noaa-gfs-standby");
    assert(!registry.setEnabled("missing-instance", true));

    return 0;
}
