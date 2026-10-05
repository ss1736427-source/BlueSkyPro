#pragma once

#include "universal_airspace_geofence_adapter.hpp"

namespace bluesky::airspace {

class AirspaceGeofenceAdapterBaseline final : public UniversalAirspaceGeofenceAdapter {
public:
    explicit AirspaceGeofenceAdapterBaseline(std::string sourceId = "SRC-11", std::string airspaceIdentity = "DEMO-AIRSPACE");

    std::string getSourceId() const override;
    std::string getAirspaceIdentity() const override;
    std::string getConfigurationVersion() const override;
    AirspaceState getState() const override;
    AirspaceQuality getQuality() const override;
    std::optional<AirspaceSample> readSample() const override;
    AirspaceResult acceptSample(const AirspaceSample& sample) override;

private:
    std::string sourceId_;
    std::string airspaceIdentity_;
    std::optional<AirspaceSample> sample_;
};

} // namespace bluesky::airspace
