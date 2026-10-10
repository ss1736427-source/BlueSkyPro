#pragma once

#include "universal_safety_failsafe_adapter.hpp"

#include <optional>
#include <string>

namespace bluesky::safety {

class SafetyFailsafeAdapterBaseline final : public UniversalSafetyFailsafeAdapter {
public:
    explicit SafetyFailsafeAdapterBaseline(std::string sourceId = "SRC-19");

    std::string getSourceId() const override;
    std::string getConfigurationVersion() const override;
    SafetyState getState() const override;
    SafetyQuality getQuality() const override;
    SafetyEvent readEvent() const override;
    SafetyResult acceptEvent(const SafetyEvent& event) override;

private:
    std::string sourceId_;
    std::optional<SafetyEvent> event_;
};

} // namespace bluesky::safety
