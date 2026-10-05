#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace bluesky::external_integration {

enum class ExternalMessageDirection { Unknown, Inbound, Outbound };
enum class ExternalMessageState { Unknown, Sent, Received, Acknowledged, Rejected, Timeout, Recovered };
enum class ExternalIntegrationQuality { Unknown, Invalid, Valid, Stale, Degraded };
enum class ExternalIntegrationError { None, InvalidData, StaleData, ConfigurationMismatch };

struct ExternalIntegrationMessage {
    std::string sourceId;
    std::string interfaceId;
    std::string messageId;
    std::string messageType;
    std::int64_t timestampMs{0};
    ExternalMessageDirection direction{ExternalMessageDirection::Unknown};
    ExternalMessageState state{ExternalMessageState::Unknown};
    std::string payloadReference;
    std::uint64_t payloadSizeBytes{0};
    ExternalIntegrationQuality quality{ExternalIntegrationQuality::Unknown};
    bool valid{false};
    bool stale{false};
};

struct ExternalIntegrationResult {
    bool accepted{false};
    ExternalIntegrationError error{ExternalIntegrationError::None};
    std::string reason;
};

class UniversalExternalIntegrationAdapter {
public:
    virtual ~UniversalExternalIntegrationAdapter() = default;
    virtual std::string getSourceId() const = 0;
    virtual std::string getInterfaceId() const = 0;
    virtual std::string getConfigurationVersion() const = 0;
    virtual ExternalIntegrationQuality getQuality() const = 0;
    virtual std::optional<ExternalIntegrationMessage> readMessage() const = 0;
    virtual ExternalIntegrationResult acceptMessage(const ExternalIntegrationMessage& message) = 0;
};

} // namespace bluesky::external_integration
