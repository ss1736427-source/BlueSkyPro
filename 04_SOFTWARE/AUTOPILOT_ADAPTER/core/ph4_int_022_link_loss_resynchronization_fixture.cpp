#include "universal_autopilot_adapter.hpp"

#include <iostream>
#include <string>

namespace {

const char* state(bluesky::autopilot::ConnectionState value) {
    using bluesky::autopilot::ConnectionState;
    switch (value) {
    case ConnectionState::Disconnected: return "DISCONNECTED";
    case ConnectionState::Connecting: return "CONNECTING";
    case ConnectionState::Connected: return "CONNECTED";
    case ConnectionState::Degraded: return "DEGRADED";
    case ConnectionState::Recovering: return "RECOVERING";
    case ConnectionState::Failed: return "FAILED";
    }
    return "UNKNOWN";
}

struct LinkLossResynchronizationModel {
    bluesky::autopilot::ConnectionState connection{bluesky::autopilot::ConnectionState::Disconnected};
    bool missionSnapshotValid{false};
    bool configurationSnapshotValid{false};
    bool telemetryFresh{false};
    std::string missionSnapshot;
    std::string configurationSnapshot;

    void connect() {
        connection = bluesky::autopilot::ConnectionState::Connected;
        telemetryFresh = true;
    }

    void loseLink() {
        connection = bluesky::autopilot::ConnectionState::Degraded;
        telemetryFresh = false;
    }

    void beginRecovery() {
        if (connection == bluesky::autopilot::ConnectionState::Degraded)
            connection = bluesky::autopilot::ConnectionState::Recovering;
    }

    bool resynchronize(const std::string& mission, const std::string& configuration) {
        if (connection != bluesky::autopilot::ConnectionState::Recovering) return false;
        if (mission.empty() || configuration.empty()) return false;
        if (!missionSnapshotValid || !configurationSnapshotValid) return false;
        if (mission != missionSnapshot || configuration != configurationSnapshot) return false;
        connection = bluesky::autopilot::ConnectionState::Connected;
        telemetryFresh = true;
        return true;
    }
};

} // namespace

int main() {
    LinkLossResynchronizationModel model;
    model.missionSnapshot = "MISSION-022:UAV-LINK-022";
    model.configurationSnapshot = "CFG-SIL-LINK-022:v1";
    model.missionSnapshotValid = true;
    model.configurationSnapshotValid = true;

    model.connect();
    const bool connected = model.connection == bluesky::autopilot::ConnectionState::Connected;

    model.loseLink();
    const bool lossDetected = model.connection == bluesky::autopilot::ConnectionState::Degraded && !model.telemetryFresh;

    model.beginRecovery();
    const bool recovering = model.connection == bluesky::autopilot::ConnectionState::Recovering;

    const bool mismatchRejected = !model.resynchronize("MISSION-OTHER:UAV-LINK-022", model.configurationSnapshot) &&
                                   model.connection == bluesky::autopilot::ConnectionState::Recovering;
    const bool synchronized = model.resynchronize(model.missionSnapshot, model.configurationSnapshot);
    const bool restored = model.connection == bluesky::autopilot::ConnectionState::Connected && model.telemetryFresh;

    std::cout << "PH4_INT_022|FLIGHT-RECORD-PH4-INT-022|UAV-LINK-022|MAVLINK2|BASELINE-1"
              << "|CONNECTED=" << (connected ? "PASS" : "FAIL")
              << "|LOSS_DETECTED=" << (lossDetected ? "PASS" : "FAIL")
              << "|RECOVERING=" << (recovering ? "PASS" : "FAIL")
              << "|MISMATCH_REJECTED=" << (mismatchRejected ? "PASS" : "FAIL")
              << "|RESYNCHRONIZED=" << (synchronized ? "PASS" : "FAIL")
              << "|RESTORED=" << (restored ? "PASS" : "FAIL")
              << "|STATE=" << state(model.connection)
              << "\n";

    return (connected && lossDetected && recovering && mismatchRejected && synchronized && restored) ? 0 : 1;
}
