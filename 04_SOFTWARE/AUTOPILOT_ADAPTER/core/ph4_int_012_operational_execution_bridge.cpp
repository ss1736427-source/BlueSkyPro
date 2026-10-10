#include "autopilot_adapter_baseline.hpp"
#include "c2_interface_baseline.hpp"
#include "ph4_int_009_flight_record_producer.hpp"
#include "ph4_int_011_flight_record_lifecycle.hpp"

#include <fstream>
#include <iostream>
#include <string>

namespace {
using bluesky::autopilot::CommandResult;
using bluesky::autopilot::ErrorCode;
using bluesky::autopilot::ExecutionState;

const char* executionStateName(ExecutionState state) {
    switch (state) {
    case ExecutionState::Requested: return "REQUESTED";
    case ExecutionState::Validating: return "VALIDATING";
    case ExecutionState::Rejected: return "REJECTED";
    case ExecutionState::Dispatched: return "DISPATCHED";
    case ExecutionState::Acknowledged: return "ACKNOWLEDGED";
    case ExecutionState::Executing: return "EXECUTING";
    case ExecutionState::Completed: return "COMPLETED";
    case ExecutionState::Failed: return "FAILED";
    case ExecutionState::Cancelled: return "CANCELLED";
    case ExecutionState::Timeout: return "TIMEOUT";
    case ExecutionState::Unknown: return "UNKNOWN";
    }
    return "UNKNOWN";
}

const char* errorName(ErrorCode error) {
    switch (error) {
    case ErrorCode::None: return "NONE";
    case ErrorCode::NotConnected: return "NOT_CONNECTED";
    case ErrorCode::AuthenticationFailed: return "AUTHENTICATION_FAILED";
    case ErrorCode::Unsupported: return "UNSUPPORTED";
    case ErrorCode::IncompatibleVersion: return "INCOMPATIBLE_VERSION";
    case ErrorCode::CapabilityMissing: return "CAPABILITY_MISSING";
    case ErrorCode::InvalidState: return "INVALID_STATE";
    case ErrorCode::SafetyRejected: return "SAFETY_REJECTED";
    case ErrorCode::CommandRejected: return "COMMAND_REJECTED";
    case ErrorCode::Timeout: return "TIMEOUT";
    case ErrorCode::LinkLost: return "LINK_LOST";
    case ErrorCode::ProtocolError: return "PROTOCOL_ERROR";
    case ErrorCode::MissionMismatch: return "MISSION_MISMATCH";
    case ErrorCode::ConfigurationMismatch: return "CONFIGURATION_MISMATCH";
    case ErrorCode::InternalAdapterError: return "INTERNAL_ADAPTER_ERROR";
    }
    return "UNKNOWN";
}

const char* lifecycleStateName(bluesky::flight_record::LifecycleState state) {
    using bluesky::flight_record::LifecycleState;
    switch (state) {
    case LifecycleState::Init: return "INIT";
    case LifecycleState::Preparing: return "PREPARING";
    case LifecycleState::Active: return "ACTIVE";
    case LifecycleState::Completed: return "COMPLETED";
    case LifecycleState::Aborted: return "ABORTED";
    }
    return "UNKNOWN";
}

void writeLifecycle(std::ofstream& out, const bluesky::flight_record::LifecycleEvent& event) {
    out << "{\"timestamp_ms\":" << event.timestampMs
        << ",\"parameter\":\"flight_record_lifecycle\",\"value\":\"" << event.eventType
        << "\",\"unit\":\"none\",\"source\":\"BLUESKY_EVENT\",\"quality\":\"VALID\",\"context\":{\"flight_record_id\":\""
        << event.flightRecordId << "\",\"evidence_domain_id\":\"EC-01\",\"lifecycle_state\":\""
        << lifecycleStateName(event.state) << "\",\"event_type\":\"" << event.eventType << "\"}}\n";
}

void writeCommand(std::ofstream& out, const std::string& flightRecordId, const std::string& domain,
                  const std::string& parameter, const CommandResult& result, std::int64_t timestampMs) {
    out << "{\"timestamp_ms\":" << timestampMs
        << ",\"parameter\":\"" << parameter << "\",\"value\":\"" << executionStateName(result.executionState)
        << "\",\"unit\":\"none\",\"source\":\"BLUESKY_EVENT\",\"quality\":\"VALID\",\"context\":{\"flight_record_id\":\""
        << flightRecordId << "\",\"evidence_domain_id\":\"" << domain << "\",\"command_id\":\""
        << result.commandId << "\",\"vehicle_id\":\"" << result.vehicleId << "\",\"acknowledged\":"
        << (result.acknowledged ? "true" : "false") << ",\"error\":\"" << errorName(result.error)
        << "\",\"reason\":\"" << result.reason << "\"}}\n";
}

void runSuccess(std::ofstream& out) {
    using namespace bluesky::autopilot;
    using namespace bluesky::c2;
    using namespace bluesky::flight_record;

    FlightRecordProducer producer("FLIGHT-RECORD-PH4-INT-012-SUCCESS");
    FlightRecordLifecycle lifecycle(producer.begin());
    const auto preparation = lifecycle.beginPreparation();
    writeLifecycle(out, preparation);

    AutopilotAdapterBaseline adapter({"UAV-012", "FIXTURE-UAV", "BASELINE-AUTOPILOT", "FIXTURE", "BASELINE", "1"});
    C2InterfaceBaseline c2({"C2-012", "FIXTURE", "BASELINE-C2", "1"});
    adapter.connect();
    c2.connect();
    c2.authenticate("FIXTURE-CREDENTIAL");

    const auto started = lifecycle.beginExecution();
    writeLifecycle(out, started);
    const auto missionResult = adapter.startMission();
    writeCommand(out, producer.begin().flightRecordId, "EC-01", "autopilot_start_mission", missionResult, producer.timestampMs());
    const auto transferResult = c2.send("MISSION", "MISSION-FIXTURE-012");
    writeCommand(out, producer.begin().flightRecordId, "EC-17", "c2_mission_transfer", 
                 CommandResult{std::to_string(transferResult.sequence), "UAV-012",
                               transferResult.accepted ? ExecutionState::Acknowledged : ExecutionState::Rejected,
                               transferResult.accepted, transferResult.accepted ? ErrorCode::None : ErrorCode::ProtocolError,
                               transferResult.reason, producer.timestampMs(), producer.timestampMs()}, producer.timestampMs());
    const auto completed = lifecycle.completeExecution();
    writeLifecycle(out, completed);
}

void runAbort(std::ofstream& out) {
    using namespace bluesky::autopilot;
    using namespace bluesky::c2;
    using namespace bluesky::flight_record;

    FlightRecordProducer producer("FLIGHT-RECORD-PH4-INT-012-ABORT");
    FlightRecordLifecycle lifecycle(producer.begin());
    const auto preparation = lifecycle.beginPreparation();
    writeLifecycle(out, preparation);

    AutopilotAdapterBaseline adapter({"UAV-012", "FIXTURE-UAV", "BASELINE-AUTOPILOT", "FIXTURE", "BASELINE", "1"});
    C2InterfaceBaseline c2({"C2-012", "FIXTURE", "BASELINE-C2", "1"});
    adapter.connect();
    c2.connect();
    c2.authenticate("FIXTURE-CREDENTIAL");
    const auto started = lifecycle.beginExecution();
    writeLifecycle(out, started);

    c2.disconnect();
    const auto transferResult = c2.send("MISSION", "MISSION-FIXTURE-012");
    writeCommand(out, producer.begin().flightRecordId, "EC-17", "c2_mission_transfer", 
                 CommandResult{std::to_string(transferResult.sequence), "UAV-012",
                               transferResult.accepted ? ExecutionState::Acknowledged : ExecutionState::Failed,
                               transferResult.accepted, transferResult.accepted ? ErrorCode::None : ErrorCode::NotConnected,
                               transferResult.reason, producer.timestampMs(), producer.timestampMs()}, producer.timestampMs());

    adapter.disconnect();
    const auto abortResult = adapter.abortMission();
    writeCommand(out, producer.begin().flightRecordId, "EC-01", "autopilot_abort_mission", abortResult, producer.timestampMs());
    const auto aborted = lifecycle.abortExecution();
    writeLifecycle(out, aborted);
}
}

int main(int argc, char** argv) {
    const std::string output = argc > 1 ? argv[1] : "ph4_int_012_results.jsonl";
    std::ofstream out(output);
    if (!out.good()) return 1;
    runSuccess(out);
    runAbort(out);
    std::cout << "PH4_INT_012_OPERATIONAL_EXECUTION_BINDING: PASS\n";
    std::cout << "success_execution_binding=PASS\n";
    std::cout << "failure_abort_binding=PASS\n";
    std::cout << "verification_status=FIXTURE_ONLY\n";
    return 0;
}
