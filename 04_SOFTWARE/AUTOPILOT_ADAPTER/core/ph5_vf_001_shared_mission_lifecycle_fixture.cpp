#include "autopilot_adapter_baseline.hpp"
#include "ph4_int_014_sil_execution_source.hpp"
#include "../../PLANNING/mission_lifecycle.hpp"
#include "../../PLANNING/model/mission_model.hpp"
#include "../../PLANNING/model/vehicle_equipment_capability.hpp"

#include <cassert>
#include <iostream>
#include <string>

using namespace bluesky::planning;
using namespace bluesky::autopilot;

namespace {
constexpr const char *kMissionId = "VF-MISSION-PH5-001";
constexpr const char *kVehicleId = "UAV-SIL-M3E-FIXTURE";
constexpr const char *kConfigurationId = "M3E-CFG-FIXTURE-001";
constexpr const char *kConfigurationRevision = "1";
constexpr const char *kPerformanceProfileId = "SIL-FIXTURE-PERF-NOT-OPERATIONAL";
constexpr const char *kPerformanceProfileRevision = "1";
constexpr const char *kRouteId = "VF-ROUTE-FIXTURE-001";
constexpr const char *kRouteVersion = "1";
constexpr const char *kInputVersion = "VF-INPUT-FIXTURE-001";

MissionLifecycleValidationEvidence validation(const std::string &id,
                                               const std::string &snapshot = {},
                                               const std::string &configuration = {}) {
    MissionLifecycleValidationEvidence value;
    value.result_id = id;
    value.mission_id = kMissionId;
    value.mission_version = "1";
    value.route_id = kRouteId;
    value.route_version = kRouteVersion;
    value.calculation_input_version = kInputVersion;
    value.validator_id = "SIL-FIXTURE-" + id;
    value.validator_version = "1";
    value.source_reference = id == "FINAL-INTEGRITY" ? id : "SIL-EVIDENCE:" + id;
    value.snapshot_id = snapshot;
    value.vehicle_configuration_id = configuration;
    value.status = MissionLifecycleValidationStatus::Passed;
    return value;
}

MissionLifecycleEvidence fixtureEvidence() {
    MissionLifecycleEvidence e;
    e.execution_mode = MissionExecutionMode::Virtual;
    e.mission_id = kMissionId;
    e.mission_version = "1";
    e.route_id = kRouteId;
    e.route_version = kRouteVersion;
    e.calculation_input_version = kInputVersion;
    e.vehicle_configuration_id = kConfigurationId;
    e.vehicle_id = kVehicleId;
    e.vehicle_source_provenance = "SIL_FIXTURE";
    e.reference_model_id = "DJI-MAVIC-3E-CATALOGUE-REFERENCE-ONLY";
    e.reference_type_id = "MULTIROTOR";
    e.vehicle_configuration_revision = kConfigurationRevision;
    e.fleet_assignment_snapshot_id = "SIL-FLEET-SNAPSHOT-001";
    e.performance_profile_id = kPerformanceProfileId;
    e.performance_profile_revision = kPerformanceProfileRevision;
    e.terrain_snapshot_id = "SIL-TERRAIN-SNAPSHOT-001";
    e.mission_notam_snapshot_id = "SIL-NOTAM-SNAPSHOT-001";
    e.weather_snapshot_id = "SIL-WEATHER-SNAPSHOT-001";
    e.notam_recheck_snapshot_id = "SIL-NOTAM-RECHECK-001";

    e.task_definition_complete = true;
    e.mission_geometry_complete = true;
    e.vehicle_configuration_validated = true;
    e.equipment_configuration_validated = true;
    e.mission_notam_snapshot_current = true;
    e.terrain_snapshot_current = true;
    e.baseline_route_generated = true;
    e.weather_assessment_complete = true;
    e.weather_aware_route_generated = true;
    e.notam_recheck_complete = true;
    e.route_candidate_selected = true;

    e.route_structure_validation = validation("ROUTE-STRUCTURE");
    e.trajectory_geometry_validation = validation("TRAJECTORY-GEOMETRY");
    e.final_planning_integrity_validation = validation("FINAL-INTEGRITY", {}, kConfigurationId);
    e.final_planning_integrity_validation.source_reference = "FINAL-INTEGRITY";
    e.notam_validation = validation("NOTAM", e.notam_recheck_snapshot_id);
    e.terrain_validation = validation("TERRAIN", e.terrain_snapshot_id);
    e.vehicle_envelope_validation = validation("VEHICLE-ENVELOPE", {}, kConfigurationId);
    e.weather_validation = validation("WEATHER", e.weather_snapshot_id);
    e.energy_reserve_validation = validation("ENERGY-RESERVE", e.weather_snapshot_id);
    e.task_quality_validation = validation("TASK-QUALITY");
    e.regulatory_validation = validation("REGULATORY");
    e.operator_review_accepted = true;

    e.regulatory_submission_prepared = false;
    e.authorization_requirement_assessed = true;
    e.authorization_required = false;
    e.authorization_not_required_reason = "VIRTUAL_SANDBOX_NO_ESORVD_SUBMISSION_OR_AUTHORIZATION";

    e.package_integrity_verified = true;
    e.package_matches_selected_route = true;
    e.package_id = "SIL-PACKAGE-001";
    e.package_route_id = kRouteId;
    e.package_route_version = kRouteVersion;
    e.package_vehicle_approved = true;
    e.package_vehicle_id = kVehicleId;
    e.package_vehicle_configuration_id = kConfigurationId;
    e.package_vehicle_configuration_revision = kConfigurationRevision;

    e.execution_session_linked = true;
    e.execution_reference_id = "SIL-EXECUTION-SESSION-001";
    e.execution_vehicle_id = kVehicleId;
    e.execution_vehicle_configuration_id = kConfigurationId;
    e.execution_vehicle_configuration_revision = kConfigurationRevision;
    e.postflight_record_id = "SIL-POSTFLIGHT-RECORD-001";
    return e;
}

MissionLifecycleTransitionResult move(MissionLifecycleRecord &record,
                                      MissionLifecycleState target,
                                      const MissionLifecycleEvidence &e,
                                      const char *reason) {
    return MissionLifecycleController::transition(
        record, target, e, "SIL-TEST-HARNESS",
        "2026-11-15T12:00:00Z", reason);
}
} // namespace

int main() {
    Identity identity;
    identity.vehicleId = kVehicleId;
    identity.vehicleType = "SIL-MULTIROTOR";
    identity.autopilot = "SIL-FCS";
    identity.firmwareVersion = "FIXTURE-1";
    identity.protocol = "MAVLINK2";
    identity.protocolVersion = "BASELINE-1";

    AutopilotAdapterBaseline adapter(identity);
    Mission mission;
    mission.identity.mission_id = kMissionId;
    mission.identity.mission_version = "1";
    mission.identity.mission_type = "VIRTUAL_FLIGHT";
    mission.identity.operational_status = "RELEASED";
    mission.vehicle_assignments.push_back({kVehicleId, kConfigurationId});

    VehicleEquipmentCapabilityProfile capabilities;
    capabilities.identity.vehicle_id = kVehicleId;
    capabilities.identity.autopilot_type = "SIL-FCS";
    capabilities.identity.autopilot_version = "FIXTURE-1";
    capabilities.configuration.configuration_version = kConfigurationRevision;
    capabilities.configuration.valid_for_mission = true;

    auto e = fixtureEvidence();
    MissionLifecycleRecord record{kMissionId, "1"};

    assert(move(record, MissionLifecycleState::Calculating, e, "SIL fixture mission admitted").accepted);
    assert(move(record, MissionLifecycleState::Validating, e, "SIL fixture route and environment validated").accepted);
    assert(move(record, MissionLifecycleState::Ready, e, "Shared lifecycle validation fixture passed").accepted);
    assert(!move(record, MissionLifecycleState::Submitted, e, "Virtual mode must not submit to ESORVD").accepted);
    assert(move(record, MissionLifecycleState::Released, e, "Virtual package released to simulator adapter without ESORVD").accepted);

    assert(adapter.connect());
    const auto compiled = adapter.compileMission(mission, capabilities);
    assert(compiled.has_value());
    assert(adapter.uploadMission(compiled.value()));
    const auto readBack = adapter.readBackMission();
    assert(readBack.has_value());
    assert(adapter.compareMission(compiled.value(), readBack.value()).equal);

    const auto start = adapter.startMission();
    assert(start.acknowledged);
    assert(start.vehicleId == kVehicleId);
    SilExecutionSource silSource("SIL-FLIGHT-RECORD-PH5-001");
    const auto executingEvent = silSource.emit(kVehicleId, start.commandId, "EXECUTING");
    assert(executingEvent.source == "SIL_EXECUTION_SOURCE");
    assert(executingEvent.flightRecordId == "SIL-FLIGHT-RECORD-PH5-001");

    e.execution_reference_id = executingEvent.flightRecordId;
    assert(move(record, MissionLifecycleState::Executing, e, "SIL adapter acknowledged execution").accepted);

    const auto log = adapter.acquireFlightLog("SIL-LOG-PH5-001");
    assert(log.accepted && log.executionState == ExecutionState::Completed);
    const auto close = adapter.close();
    assert(close.acknowledged && close.executionState == ExecutionState::Completed);
    const auto completedEvent = silSource.emit(kVehicleId, start.commandId, "COMPLETED");
    assert(completedEvent.flightRecordId == executingEvent.flightRecordId);

    e.execution_closed = true;
    e.postflight_data_archived = true;
    e.postflight_record_id = log.sourceReference;
    assert(move(record, MissionLifecycleState::Completed, e, "SIL log archived and adapter closed").accepted);

    assert(record.state == MissionLifecycleState::Completed);
    assert(record.history.size() == 6);
    for (const auto &event : record.history) {
        assert(event.vehicle_id == kVehicleId);
        assert(event.execution_mode == MissionExecutionMode::Virtual);
        assert(event.vehicle_source_provenance == "SIL_FIXTURE");
        assert(event.reference_model_id == e.reference_model_id);
        assert(event.performance_profile_id == kPerformanceProfileId);
    }

    std::cout << "PH5_VF_001|SHARED_MISSION_LIFECYCLE=PASS"
              << "|SIL_ADAPTER_READBACK=PASS|SIL_EXECUTION_EVIDENCE=PASS"
              << "|POSTFLIGHT_ARCHIVE=PASS|FINAL_STATE=COMPLETED"
              << "|EVIDENCE_CLASS=SIL_FIXTURE_ONLY" << std::endl;
    return 0;
}
