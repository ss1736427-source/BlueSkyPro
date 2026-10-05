#pragma once

#include "universal_propulsion_adapter.hpp"

namespace bluesky::propulsion {

class PropulsionAdapterBaseline final : public UniversalPropulsionAdapter {
public:
    explicit PropulsionAdapterBaseline(std::string sourceId = "SRC-09", std::string propulsionIdentity = "DEMO-PROPULSION");

    std::string getSourceId() const override;
    std::string getPropulsionIdentity() const override;
    std::string getConfigurationVersion() const override;
    PropulsionState getState() const override;
    PropulsionQuality getQuality() const override;
    std::optional<PropulsionSample> readSample() const override;
    PropulsionResult acceptSample(const PropulsionSample& sample) override;

private:
    std::string sourceId_;
    std::string propulsionIdentity_;
    std::optional<PropulsionSample> sample_;
};

} // namespace bluesky::propulsion
