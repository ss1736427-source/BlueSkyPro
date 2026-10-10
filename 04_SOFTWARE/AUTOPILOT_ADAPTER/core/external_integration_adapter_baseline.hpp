#pragma once

#include "universal_external_integration_adapter.hpp"

namespace bluesky::external_integration {

class ExternalIntegrationAdapterBaseline final : public UniversalExternalIntegrationAdapter {
public:
    explicit ExternalIntegrationAdapterBaseline(std::string sourceId = "SRC-17", std::string interfaceId = "DEMO-EXTERNAL");

    std::string getSourceId() const override;
    std::string getInterfaceId() const override;
    std::string getConfigurationVersion() const override;
    ExternalIntegrationQuality getQuality() const override;
    std::optional<ExternalIntegrationMessage> readMessage() const override;
    ExternalIntegrationResult acceptMessage(const ExternalIntegrationMessage& message) override;

private:
    std::string sourceId_;
    std::string interfaceId_;
    std::optional<ExternalIntegrationMessage> message_;
};

} // namespace bluesky::external_integration
