#include "autopilot_adapter_baseline.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>

using namespace bluesky::autopilot;

static bool contains(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

int main() {
    Identity identity{"UAV-COMPAT-026", "MULTIROTOR", "BASELINE-FCS", "FW-1.0", "MAVLINK2", "BASELINE-1"};
    AutopilotAdapterBaseline adapter(identity);
    const auto manifest = adapter.getCapabilityManifest();
    assert(manifest.adapterId == "BASELINE-FCS:MAVLINK2");
    assert(manifest.protocol == "MAVLINK2");
    assert(manifest.protocolVersion == "BASELINE-1");
    assert(manifest.fcsName == "BASELINE-FCS");
    assert(manifest.fcsVersion == "FW-1.0");
    assert(contains(manifest.vehicleClasses, "MULTIROTOR"));
    assert(contains(manifest.supportedCommands, "startMission"));
    assert(contains(manifest.supportedMissionFeatures, "mission_readback"));
    assert(contains(manifest.supportedParameterFeatures, "configuration_verify"));
    assert(contains(manifest.telemetryFeatures, "navigation"));
    assert(contains(manifest.failsafeFeatures, "failsafe_state"));
    assert(contains(manifest.logFeatures, "flight_log_acquisition"));
    assert(manifest.verificationStatus == "SIL_FIXTURE_ONLY");

    const auto compatible = adapter.checkCompatibility({"MAVLINK2", "BASELINE-1", "mission_readback"});
    assert(compatible.compatible);
    assert(compatible.state == CompatibilityState::Compatible);
    assert(compatible.error == ErrorCode::None);

    const auto protocolMismatch = adapter.checkCompatibility({"PX4-NATIVE", "BASELINE-1", "mission_readback"});
    assert(!protocolMismatch.compatible);
    assert(protocolMismatch.error == ErrorCode::IncompatibleVersion);
    assert(protocolMismatch.reason == "PROTOCOL_MISMATCH");

    const auto versionMismatch = adapter.checkCompatibility({"MAVLINK2", "2", "mission_readback"});
    assert(!versionMismatch.compatible);
    assert(versionMismatch.error == ErrorCode::IncompatibleVersion);
    assert(versionMismatch.reason == "PROTOCOL_VERSION_MISMATCH");

    const auto capabilityMissing = adapter.checkCompatibility({"MAVLINK2", "BASELINE-1", "unsupported_feature"});
    assert(!capabilityMissing.compatible);
    assert(capabilityMissing.error == ErrorCode::CapabilityMissing);
    assert(capabilityMissing.reason == "MISSION_CAPABILITY_MISSING");

    std::cout << "PH4_INT_026|FLIGHT-RECORD-PH4-INT-026|UAV-COMPAT-026|MAVLINK2|BASELINE-1"
              << "|MANIFEST=PASS|COMPATIBLE=PASS|PROTOCOL_MISMATCH=PASS|VERSION_MISMATCH=PASS"
              << "|CAPABILITY_MISSING=PASS|IDENTITY=PASS|CAPABILITY_SETS=PASS|VERIFICATION_STATUS=PASS" << std::endl;
    return 0;
}
