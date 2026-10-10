#pragma once

#include "universal_equipment_adapter.hpp"

#include <string>
#include <vector>

namespace bluesky::equipment {

class EquipmentAdapterBaseline final : public UniversalEquipmentAdapter {
public:
    EquipmentAdapterBaseline(Identity identity,
                             std::string profile,
                             std::string configuration,
                             std::vector<Capability> capabilities,
                             std::vector<DataOutput> outputs);

    Identity identify() const override;
    std::string readProfile() const override;
    std::string readConfiguration() const override;
    std::vector<Capability> readCapabilities() const override;
    EquipmentState readState() const override;
    std::optional<Telemetry> readTelemetry() const override;
    ActionResult translateAction(const std::string& action) override;
    std::vector<DataOutput> readDataOutputs() const override;

    void setState(EquipmentState state);
    void setTelemetry(Telemetry telemetry);

private:
    Identity identity_;
    std::string profile_;
    std::string configuration_;
    std::vector<Capability> capabilities_;
    std::vector<DataOutput> outputs_;
    EquipmentState state_{EquipmentState::Unknown};
    std::optional<Telemetry> telemetry_;
    std::int64_t actionSequence_{0};
};

} // namespace bluesky::equipment
