#pragma once

#include "mission_lifecycle_final_integrity_adapter.hpp"
#include "vehicle_mission_compiler.hpp"

#include <string>

namespace bluesky::planning {

struct MissionFinalizationRuntimeResult {
    VehicleCompiledProfile compiled_profile;
    MissionLifecycleValidationEvidence final_integrity_evidence;
    MissionLifecycleTransitionResult transition;
};

// Runtime owner for the final compiler -> integrity adapter -> READY gate chain.
class MissionFinalizationRuntime {
public:
    static MissionFinalizationRuntimeResult finalizeReady(
        MissionLifecycleRecord& record,
        MissionLifecycleEvidence evidence,
        const FlightProfile& selected_flight_profile,
        const VehicleCompilationInput& assigned_vehicle,
        const std::string& actor_id,
        const std::string& timestamp,
        const std::string& reason)
    {
        MissionFinalizationRuntimeResult result;
        result.compiled_profile = VehicleMissionCompiler::compile(
            selected_flight_profile, assigned_vehicle);
        result.final_integrity_evidence =
            makeFinalPlanningIntegrityValidationEvidence(
                result.compiled_profile,
                record.mission_id,
                record.mission_version,
                evidence.route_id,
                evidence.route_version,
                evidence.calculation_input_version);
        evidence.final_planning_integrity_validation =
            result.final_integrity_evidence;
        result.transition = MissionLifecycleController::transition(
            record, MissionLifecycleState::Ready, evidence,
            actor_id, timestamp, reason);
        return result;
    }
};

} // namespace bluesky::planning
