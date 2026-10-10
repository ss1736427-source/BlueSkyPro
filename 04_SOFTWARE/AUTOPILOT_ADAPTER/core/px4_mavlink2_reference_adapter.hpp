#pragma once

#include "autopilot_adapter_baseline.hpp"

namespace bluesky::autopilot {

// Reference integration boundary for PX4 over MAVLink2.
// Transport and wire-message implementation remain isolated from the
// normalized BlueSky adapter contract.
class Px4Mavlink2ReferenceAdapter final : public AutopilotAdapterBaseline {
public:
    explicit Px4Mavlink2ReferenceAdapter(Identity identity = {});

    Identity getVehicleIdentity() const override;
    std::string getAutopilotIdentity() const override;
    std::string getProtocolVersion() const override;
    Capabilities getCapabilities() const override;
};

} // namespace bluesky::autopilot
