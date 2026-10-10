#include "capability_discovery_boundary.hpp"

namespace bluesky::operations {

CapabilityDiscoveryBoundary::CapabilityDiscoveryBoundary(
    DeviceIdentityRuntime& identity_runtime,
    DeviceCapabilityRuntime& capability_runtime)
    : identity_runtime_(identity_runtime),
      capability_runtime_(capability_runtime) {}

bool CapabilityDiscoveryBoundary::valid(
    const CapabilityDiscoveryReport& report) {
    if (report.device_id.empty() ||
        report.firmware.empty() ||
        report.protocol.empty() ||
        report.protocol_version.empty() ||
        report.capabilities.empty()) {
        return false;
    }

    for (const auto& capability : report.capabilities) {
        if (capability.empty()) {
            return false;
        }
    }

    return true;
}

CapabilityDiscoveryResult CapabilityDiscoveryBoundary::ingest(
    const CapabilityDiscoveryReport& report) {
    if (!valid(report)) {
        return CapabilityDiscoveryResult::InvalidReport;
    }

    const auto identity = identity_runtime_.resolveDevice(report.device_id);
    if (!identity.has_value()) {
        return CapabilityDiscoveryResult::IdentityNotFound;
    }

    if (identity->lifecycle == DeviceIdentityLifecycle::Revoked) {
        return CapabilityDiscoveryResult::IdentityRevoked;
    }

    DeviceCapabilitySnapshot snapshot{
        report.device_id,
        identity->vehicle_id,
        identity->autopilot_id,
        report.firmware,
        report.protocol,
        report.protocol_version,
        DeviceCapabilityLifecycle::Discovered,
        report.capabilities};

    if (capability_runtime_.resolve(report.device_id, snapshot)) {
        const auto result = capability_runtime_.updateSnapshot(std::move(snapshot));
        return result == DeviceCapabilityResult::Updated
            ? CapabilityDiscoveryResult::Updated
            : CapabilityDiscoveryResult::InvalidReport;
    }

    const auto result = capability_runtime_.registerSnapshot(std::move(snapshot));
    return result == DeviceCapabilityResult::Registered
        ? CapabilityDiscoveryResult::Registered
        : CapabilityDiscoveryResult::InvalidReport;
}

} // namespace bluesky::operations
