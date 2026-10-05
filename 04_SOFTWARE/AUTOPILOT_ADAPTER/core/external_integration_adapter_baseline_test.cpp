#include "external_integration_adapter_baseline.hpp"

#include <cassert>

int main() {
    using namespace bluesky::external_integration;

    ExternalIntegrationAdapterBaseline adapter;
    assert(adapter.getSourceId() == "SRC-17");
    assert(adapter.getInterfaceId() == "DEMO-EXTERNAL");
    assert(adapter.getConfigurationVersion() == "EC-16-BASELINE-1");

    ExternalIntegrationMessage message;
    message.sourceId = "SRC-17";
    message.interfaceId = "DEMO-EXTERNAL";
    message.messageId = "MSG-001";
    message.messageType = "FLIGHT_PLAN_STATUS";
    message.timestampMs = 1700000000000;
    message.direction = ExternalMessageDirection::Outbound;
    message.state = ExternalMessageState::Acknowledged;
    message.payloadReference = "fixture://ec16/msg-001";
    message.payloadSizeBytes = 128;
    message.quality = ExternalIntegrationQuality::Valid;
    message.valid = true;

    const auto result = adapter.acceptMessage(message);
    assert(result.accepted);
    assert(adapter.readMessage().has_value());
    assert(adapter.readMessage()->messageId == "MSG-001");

    message.messageId.clear();
    assert(!adapter.acceptMessage(message).accepted);

    message.messageId = "MSG-002";
    message.stale = true;
    assert(!adapter.acceptMessage(message).accepted);

    return 0;
}
