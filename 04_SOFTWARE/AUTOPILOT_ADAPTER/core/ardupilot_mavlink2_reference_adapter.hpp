#pragma once

#include "autopilot_adapter_baseline.hpp"

namespace bluesky::autopilot {

// Reference integration boundary for ArduPilot over MAVLink2.
// Transport and wire-message implementation remain isolated from the
// normalized BlueSky adapter contract.
class ArduPilotMavlink2ReferenceAdapter final : public AutopilotAdapterBaseline {
public:
    explicit ArduPilotMavlink2ReferenceAdapter(Identity identity = {});

    Identity getVehicleIdentity() const override;
    std::string getAutopilotIdentity() const override;
    std::string getProtocolVersion() const override;
    Capabilities getCapabilities() const override;
};

} // namespace bluesky::autopilot
