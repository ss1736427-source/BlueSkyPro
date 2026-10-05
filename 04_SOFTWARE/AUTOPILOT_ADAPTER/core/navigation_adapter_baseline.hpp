#pragma once

#include "universal_navigation_adapter.hpp"

namespace bluesky::navigation {

class NavigationAdapterBaseline final : public UniversalNavigationAdapter {
public:
    explicit NavigationAdapterBaseline(std::string sourceId = "SRC-06");

    std::string getSourceId() const override;
    std::string getReceiverIdentity() const override;
    std::string getFirmwareVersion() const override;
    std::string getProtocolVersion() const override;
    NavigationQuality getQuality() const override;
    CorrectionState getCorrectionState() const override;
    std::optional<NavigationSample> readSample() const override;
    NavigationResult acceptSample(const NavigationSample& sample) override;

private:
    std::string sourceId_;
    NavigationSample sample_{};
    bool hasSample_{false};
};

} // namespace bluesky::navigation
