#include "external_integration_adapter_baseline.hpp"

#include <utility>

namespace bluesky::external_integration {

ExternalIntegrationAdapterBaseline::ExternalIntegrationAdapterBaseline(std::string sourceId, std::string interfaceId)
    : sourceId_(std::move(sourceId)), interfaceId_(std::move(interfaceId)) {}

std::string ExternalIntegrationAdapterBaseline::getSourceId() const { return sourceId_; }
std::string ExternalIntegrationAdapterBaseline::getInterfaceId() const { return interfaceId_; }
std::string ExternalIntegrationAdapterBaseline::getConfigurationVersion() const { return "EC-16-BASELINE-1"; }

ExternalIntegrationQuality ExternalIntegrationAdapterBaseline::getQuality() const {
    return message_.has_value() ? message_->quality : ExternalIntegrationQuality::Unknown;
}

std::optional<ExternalIntegrationMessage> ExternalIntegrationAdapterBaseline::readMessage() const { return message_; }

ExternalIntegrationResult ExternalIntegrationAdapterBaseline::acceptMessage(const ExternalIntegrationMessage& message) {
    if (message.sourceId != sourceId_ || message.interfaceId != interfaceId_ || message.messageId.empty() ||
        message.messageType.empty() || message.timestampMs <= 0 || !message.valid) {
        return {false, ExternalIntegrationError::InvalidData, "external integration message validation failed"};
    }
    if (message.stale) {
        return {false, ExternalIntegrationError::StaleData, "external integration message is stale"};
    }
    message_ = message;
    return {true, ExternalIntegrationError::None, "accepted"};
}

} // namespace bluesky::external_integration
