#include "parameter_configuration_runtime.hpp"

namespace bluesky::planning {

ParameterConfigurationSnapshot ParameterConfigurationRuntime::identitySnapshot(
    const bluesky::autopilot::Identity& identity,
    const std::string& configuration) {
    return {
        identity.vehicleId,
        identity.autopilot,
        identity.firmwareVersion,
        identity.protocol,
        identity.protocolVersion,
        configuration
    };
}

ParameterConfigurationResult ParameterConfigurationRuntime::reject(
    const std::string& reason) {
    ParameterConfigurationResult result;
    result.state = ParameterSyncState::Rejected;
    result.reason = reason;
    return result;
}

ParameterConfigurationResult ParameterConfigurationRuntime::read(
    bluesky::autopilot::UniversalAutopilotAdapter& adapter) {
    if (adapter.getConnectionState() != bluesky::autopilot::ConnectionState::Connected) {
        return reject("AUTOPILOT_NOT_CONNECTED");
    }

    const auto configuration = adapter.readBack();
    if (!configuration.has_value()) {
        return reject("CONFIGURATION_READBACK_EMPTY");
    }

    ParameterConfigurationResult result;
    result.state = ParameterSyncState::Read;
    result.snapshot = identitySnapshot(adapter.getVehicleIdentity(), *configuration);
    result.reason = "CONFIGURATION_READ";
    return result;
}

ParameterConfigurationResult ParameterConfigurationRuntime::compare(
    const ParameterConfigurationSnapshot& approved,
    bluesky::autopilot::UniversalAutopilotAdapter& adapter) {
    const auto current = read(adapter);
    if (current.state != ParameterSyncState::Read) {
        return current;
    }

    const auto identity = adapter.getVehicleIdentity();
    if (approved.vehicle_id != identity.vehicleId ||
        approved.autopilot != identity.autopilot ||
        approved.firmware_version != identity.firmwareVersion ||
        approved.protocol != identity.protocol ||
        approved.protocol_version != identity.protocolVersion) {
        auto result = current;
        result.state = ParameterSyncState::Failed;
        result.reason = "CONFIGURATION_IDENTITY_MISMATCH";
        result.mismatches.push_back("IDENTITY");
        return result;
    }

    auto result = current;
    result.state = ParameterSyncState::Compared;
    result.reason = "CONFIGURATION_MATCH";
    if (approved.configuration != current.snapshot.configuration) {
        result.state = ParameterSyncState::Failed;
        result.reason = "CONFIGURATION_MISMATCH";
        result.mismatches.push_back("CONFIGURATION");
    }
    return result;
}

ParameterConfigurationResult ParameterConfigurationRuntime::backup(
    bluesky::autopilot::UniversalAutopilotAdapter& adapter) {
    const auto current = read(adapter);
    if (current.state != ParameterSyncState::Read) {
        return current;
    }

    auto result = current;
    result.state = ParameterSyncState::BackedUp;
    result.backup = current.snapshot;
    result.reason = "CONFIGURATION_BACKUP_CAPTURED";
    return result;
}

ParameterConfigurationResult ParameterConfigurationRuntime::write(
    const ParameterConfigurationSnapshot& approved,
    bluesky::autopilot::UniversalAutopilotAdapter& adapter) {
    if (adapter.getConnectionState() != bluesky::autopilot::ConnectionState::Connected) {
        return reject("AUTOPILOT_NOT_CONNECTED");
    }

    const auto identity = adapter.getVehicleIdentity();
    if (approved.vehicle_id != identity.vehicleId ||
        approved.autopilot != identity.autopilot ||
        approved.firmware_version != identity.firmwareVersion ||
        approved.protocol != identity.protocol ||
        approved.protocol_version != identity.protocolVersion) {
        return reject("CONFIGURATION_IDENTITY_MISMATCH");
    }
    if (approved.configuration.empty()) {
        return reject("CONFIGURATION_REQUIRED");
    }

    const auto validation = adapter.validate(approved.configuration);
    if (!validation.acknowledged) {
        ParameterConfigurationResult result;
        result.state = ParameterSyncState::Rejected;
        result.snapshot = approved;
        result.reason = validation.reason.empty() ? "CONFIGURATION_VALIDATION_FAILED" : validation.reason;
        return result;
    }

    const auto writeResult = adapter.write(approved.configuration);
    if (!writeResult.acknowledged) {
        ParameterConfigurationResult result;
        result.state = ParameterSyncState::Failed;
        result.snapshot = approved;
        result.reason = writeResult.reason.empty() ? "CONFIGURATION_WRITE_FAILED" : writeResult.reason;
        return result;
    }

    ParameterConfigurationResult result;
    result.state = ParameterSyncState::Written;
    result.snapshot = approved;
    result.reason = "CONFIGURATION_WRITTEN";
    return result;
}

ParameterConfigurationResult ParameterConfigurationRuntime::restore(
    const ParameterConfigurationSnapshot& backup,
    bluesky::autopilot::UniversalAutopilotAdapter& adapter) {
    auto result = write(backup, adapter);
    if (result.state == ParameterSyncState::Written) {
        result.state = ParameterSyncState::Restored;
        result.reason = "CONFIGURATION_RESTORED";
    }
    result.backup = backup;
    return result;
}

ParameterConfigurationResult ParameterConfigurationRuntime::synchronize(
    const ParameterConfigurationSnapshot& approved,
    bluesky::autopilot::UniversalAutopilotAdapter& adapter) {
    const auto current = read(adapter);
    if (current.state != ParameterSyncState::Read) {
        return current;
    }

    const auto comparison = compare(approved, adapter);
    if (comparison.state == ParameterSyncState::Compared) {
        auto result = comparison;
        result.backup = current.snapshot;
        result.state = ParameterSyncState::Verified;
        result.reason = "CONFIGURATION_ALREADY_SYNCHRONIZED";
        return result;
    }
    if (comparison.reason != "CONFIGURATION_MISMATCH") {
        return comparison;
    }

    const auto written = write(approved, adapter);
    if (written.state != ParameterSyncState::Written) {
        auto result = written;
        result.backup = current.snapshot;
        return result;
    }

    auto verified = verify(approved, adapter);
    verified.backup = current.snapshot;
    return verified;
}

ParameterConfigurationResult ParameterConfigurationRuntime::verify(
    const ParameterConfigurationSnapshot& approved,
    bluesky::autopilot::UniversalAutopilotAdapter& adapter) {
    const auto comparison = compare(approved, adapter);
    if (comparison.state == ParameterSyncState::Compared) {
        auto result = comparison;
        result.state = ParameterSyncState::Verified;
        result.reason = "CONFIGURATION_VERIFIED";
        return result;
    }

    auto result = comparison;
    result.state = ParameterSyncState::Failed;
    if (result.reason.empty()) {
        result.reason = "CONFIGURATION_VERIFY_FAILED";
    }
    return result;
}

} // namespace bluesky::planning
