#pragma once

#include <string>
#include <vector>

namespace bluesky::operations {

enum class MissionObjectiveProfile {
    SurveyMapping,
    Reconstruction3D,
    SearchReconnaissance,
    Delivery,
    Inspection,
    Monitoring,
    LongEnduranceBvlos,
    TimeCriticalArrival,
    MultiUavCoordinated
};

enum class ObjectivePriority {
    HardAdmissibility,
    Primary,
    Secondary,
    TieBreaker
};

struct ObjectiveProfileDefinition {
    MissionObjectiveProfile profile;
    const char* primary_objective;
    std::vector<std::string> secondary_objectives;
};

struct ObjectiveProfileSelectionContext {
    std::string task_type;
    bool multi_uav{false};
    bool time_critical{false};
    bool long_endurance_or_bvlos{false};
};

inline const ObjectiveProfileDefinition& objective_profile(
    MissionObjectiveProfile profile) {
    static const ObjectiveProfileDefinition survey{
        MissionObjectiveProfile::SurveyMapping,
        "required coverage and data quality",
        {"coverage completeness", "GSD", "overlap", "flight geometry", "sensor constraints", "energy efficiency", "time"}};
    static const ObjectiveProfileDefinition reconstruction{
        MissionObjectiveProfile::Reconstruction3D,
        "reconstruction quality and geometric completeness",
        {"surface/volume coverage", "observation geometry", "overlap", "multi-angle acquisition", "occlusion reduction", "sensor constraints", "energy", "time"}};
    static const ObjectiveProfileDefinition search{
        MissionObjectiveProfile::SearchReconnaissance,
        "probability and completeness of detection/observation",
        {"search coverage", "sensor performance", "observation geometry", "revisit/uncertainty reduction", "energy reserve", "C2 continuity", "time"}};
    static const ObjectiveProfileDefinition delivery{
        MissionObjectiveProfile::Delivery,
        "successful delivery to the required location/condition",
        {"delivery feasibility", "route safety", "energy reserve", "equipment constraints", "ETA", "propulsion resource", "route efficiency"}};
    static const ObjectiveProfileDefinition inspection{
        MissionObjectiveProfile::Inspection,
        "acquisition of the required inspection information",
        {"required viewpoints", "sensor geometry", "resolution/quality", "surface completeness", "obstacle/clearance constraints", "energy", "time"}};
    static const ObjectiveProfileDefinition monitoring{
        MissionObjectiveProfile::Monitoring,
        "continuity and quality of observation",
        {"observation continuity", "required sensor geometry", "revisit interval", "C2 continuity", "energy reserve", "time/resource efficiency"}};
    static const ObjectiveProfileDefinition endurance{
        MissionObjectiveProfile::LongEnduranceBvlos,
        "robust completion of the mission with adequate energy and operational margins",
        {"energy reserve", "route robustness", "C2 availability", "recovery feasibility", "propulsion resource", "mission completion", "ETA"}};
    static const ObjectiveProfileDefinition arrival{
        MissionObjectiveProfile::TimeCriticalArrival,
        "achievement of the required arrival window",
        {"ETA accuracy", "wind robustness", "energy reserve", "route safety", "propulsion/resource constraints"}};
    static const ObjectiveProfileDefinition coordinated{
        MissionObjectiveProfile::MultiUavCoordinated,
        "successful coordinated completion of the parent task",
        {"task completeness", "temporal/spatial coordination", "conflict avoidance", "individual UAV energy margins", "heterogeneous vehicle capabilities", "C2 continuity", "overall mission time"}};

    switch (profile) {
    case MissionObjectiveProfile::SurveyMapping: return survey;
    case MissionObjectiveProfile::Reconstruction3D: return reconstruction;
    case MissionObjectiveProfile::SearchReconnaissance: return search;
    case MissionObjectiveProfile::Delivery: return delivery;
    case MissionObjectiveProfile::Inspection: return inspection;
    case MissionObjectiveProfile::Monitoring: return monitoring;
    case MissionObjectiveProfile::LongEnduranceBvlos: return endurance;
    case MissionObjectiveProfile::TimeCriticalArrival: return arrival;
    case MissionObjectiveProfile::MultiUavCoordinated: return coordinated;
    }
    return survey;
}

inline MissionObjectiveProfile select_objective_profile(
    const ObjectiveProfileSelectionContext& context) {
    if (context.multi_uav) return MissionObjectiveProfile::MultiUavCoordinated;
    if (context.time_critical) return MissionObjectiveProfile::TimeCriticalArrival;
    if (context.long_endurance_or_bvlos) return MissionObjectiveProfile::LongEnduranceBvlos;

    if (context.task_type == "survey" || context.task_type == "mapping")
        return MissionObjectiveProfile::SurveyMapping;
    if (context.task_type == "3d" || context.task_type == "reconstruction")
        return MissionObjectiveProfile::Reconstruction3D;
    if (context.task_type == "search" || context.task_type == "reconnaissance")
        return MissionObjectiveProfile::SearchReconnaissance;
    if (context.task_type == "delivery") return MissionObjectiveProfile::Delivery;
    if (context.task_type == "inspection") return MissionObjectiveProfile::Inspection;
    if (context.task_type == "monitoring") return MissionObjectiveProfile::Monitoring;

    return MissionObjectiveProfile::SurveyMapping;
}

inline bool hard_admissibility_before_objective_evaluation() {
    return true;
}

} // namespace bluesky::operations
