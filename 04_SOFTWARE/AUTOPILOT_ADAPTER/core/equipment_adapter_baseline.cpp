#include "equipment_adapter_baseline.hpp"

#include <utility>

namespace bluesky::equipment {

EquipmentAdapterBaseline::EquipmentAdapterBaseline(Identity identity,
                                                   std::string profile,
                                                   std::string configuration,
                                                   std::vector<Capability> capabilities,
                                                   std::vector<DataOutput> outputs)
    : identity_(std::move(identity)),
      profile_(std::move(profile)),
      configuration_(std::move(configuration)),
      capabilities_(std::move(capabilities)),
      outputs_(std::move(outputs)),
      state_(EquipmentState::Off) {}

Identity EquipmentAdapterBaseline::identify() const { return identity_; }

std::string EquipmentAdapterBaseline::readProfile() const { return profile_; }

std::string EquipmentAdapterBaseline::readConfiguration() const { return configuration_; }

std::vector<Capability> EquipmentAdapterBaseline::readCapabilities() const { return capabilities_; }

EquipmentState EquipmentAdapterBaseline::readState() const { return state_; }

std::optional<Telemetry> EquipmentAdapterBaseline::readTelemetry() const { return telemetry_; }

ActionResult EquipmentAdapterBaseline::translateAction(const std::string& action) {
    const auto now = telemetry_.has_value() ? telemetry_->adapterTimestampMs : 0;
    const auto source = telemetry_.has_value() ? telemetry_->sourceTimestampMs : 0;
    const std::string actionId = "ACT-" + std::to_string(++actionSequence_);

    for (const auto& capability : capabilities_) {
        if (capability.id == action) {
            if (capability.availability == AvailabilityState::Supported ||
                capability.availability == AvailabilityState::Verified) {
                return {actionId, identity_.equipmentId, true, ErrorCode::None, "", source, now};
            }
            return {actionId, identity_.equipmentId, false, ErrorCode::CapabilityMissing,
                    "capability is not currently available", source, now};
        }
    }

    return {actionId, identity_.equipmentId, false, ErrorCode::Unsupported,
            "unsupported action", source, now};
}

std::vector<DataOutput> EquipmentAdapterBaseline::readDataOutputs() const { return outputs_; }

void EquipmentAdapterBaseline::setState(EquipmentState state) { state_ = state; }

void EquipmentAdapterBaseline::setTelemetry(Telemetry telemetry) { telemetry_ = std::move(telemetry); }

} // namespace bluesky::equipment
