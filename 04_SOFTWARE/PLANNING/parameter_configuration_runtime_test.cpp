#include "parameter_configuration_runtime.hpp"
#include "../AUTOPILOT_ADAPTER/core/autopilot_adapter_baseline.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <iostream>

namespace {

bluesky::autopilot::AutopilotAdapterBaseline makeAdapter() {
    bluesky::autopilot::Identity identity;
    identity.vehicleId = "UAV-021";
    identity.vehicleType = "TEST";
    identity.autopilot = "TEST-FCS";
    identity.firmwareVersion = "FW-1";
    identity.protocol = "MAVLink2";
    identity.protocolVersion = "2";
    return bluesky::autopilot::AutopilotAdapterBaseline(identity);
}

bluesky::planning::ParameterConfigurationSnapshot makeApproved() {
    return {
        "UAV-021",
        "TEST-FCS",
        "FW-1",
        "MAVLink2",
        "2",
        "PARAM_A=10;PARAM_B=20"
    };
}

void testReadWriteCompareVerify() {
    auto adapter = makeAdapter();
    assert(adapter.connect());

    const auto approved = makeApproved();
    const auto write = bluesky::planning::ParameterConfigurationRuntime::write(approved, adapter);
    assert(write.state == bluesky::planning::ParameterSyncState::Written);

    const auto read = bluesky::planning::ParameterConfigurationRuntime::read(adapter);
    assert(read.state == bluesky::planning::ParameterSyncState::Read);
    assert(read.snapshot.configuration == approved.configuration);

    const auto compare = bluesky::planning::ParameterConfigurationRuntime::compare(approved, adapter);
    assert(compare.state == bluesky::planning::ParameterSyncState::Compared);
    assert(compare.reason == "CONFIGURATION_MATCH");

    const auto verify = bluesky::planning::ParameterConfigurationRuntime::verify(approved, adapter);
    assert(verify.state == bluesky::planning::ParameterSyncState::Verified);
    assert(verify.reason == "CONFIGURATION_VERIFIED");
}

void testMismatchIsExplicit() {
    auto adapter = makeAdapter();
    assert(adapter.connect());

    const auto approved = makeApproved();
    assert(adapter.write("PARAM_A=99").acknowledged);

    const auto compare =
        bluesky::planning::ParameterConfigurationRuntime::compare(approved, adapter);
    assert(compare.state == bluesky::planning::ParameterSyncState::Failed);
    assert(compare.reason == "CONFIGURATION_MISMATCH");
    assert(!compare.mismatches.empty());
}

void testBackupAndRestore() {
    auto adapter = makeAdapter();
    assert(adapter.connect());

    const auto original = makeApproved();
    assert(adapter.write(original.configuration).acknowledged);

    const auto backup =
        bluesky::planning::ParameterConfigurationRuntime::backup(adapter);
    assert(backup.state == bluesky::planning::ParameterSyncState::BackedUp);
    assert(backup.backup.has_value());
    assert(backup.backup->configuration == original.configuration);

    auto changed = original;
    changed.configuration = "PARAM_A=50;PARAM_B=60";
    assert(adapter.write(changed.configuration).acknowledged);

    const auto restore =
        bluesky::planning::ParameterConfigurationRuntime::restore(*backup.backup, adapter);
    assert(restore.state == bluesky::planning::ParameterSyncState::Restored);

    const auto verify =
        bluesky::planning::ParameterConfigurationRuntime::verify(original, adapter);
    assert(verify.state == bluesky::planning::ParameterSyncState::Verified);
}

void testSynchronizeChangesOnlyWhenNeeded() {
    auto adapter = makeAdapter();
    assert(adapter.connect());

    const auto approved = makeApproved();
    assert(adapter.write("PARAM_A=1;PARAM_B=2").acknowledged);

    const auto sync =
        bluesky::planning::ParameterConfigurationRuntime::synchronize(approved, adapter);
    assert(sync.state == bluesky::planning::ParameterSyncState::Verified);
    assert(sync.reason == "CONFIGURATION_VERIFIED");
    assert(sync.backup.has_value());
    assert(sync.backup->configuration == "PARAM_A=1;PARAM_B=2");

    const auto already =
        bluesky::planning::ParameterConfigurationRuntime::synchronize(approved, adapter);
    assert(already.state == bluesky::planning::ParameterSyncState::Verified);
    assert(already.reason == "CONFIGURATION_ALREADY_SYNCHRONIZED");
}

void testRejectsIdentityMismatchAndDisconnected() {
    auto adapter = makeAdapter();
    const auto approved = makeApproved();

    const auto disconnected =
        bluesky::planning::ParameterConfigurationRuntime::read(adapter);
    assert(disconnected.state == bluesky::planning::ParameterSyncState::Rejected);
    assert(disconnected.reason == "AUTOPILOT_NOT_CONNECTED");

    assert(adapter.connect());
    auto wrong = approved;
    wrong.vehicle_id = "UAV-OTHER";

    const auto mismatch =
        bluesky::planning::ParameterConfigurationRuntime::write(wrong, adapter);
    assert(mismatch.state == bluesky::planning::ParameterSyncState::Rejected);
    assert(mismatch.reason == "CONFIGURATION_IDENTITY_MISMATCH");
}

} // namespace

int main() {
    testReadWriteCompareVerify();
    testMismatchIsExplicit();
    testBackupAndRestore();
    testSynchronizeChangesOnlyWhenNeeded();
    testRejectsIdentityMismatchAndDisconnected();
    std::cout << "parameter_configuration_runtime_test: PASS\n";
    return 0;
}
