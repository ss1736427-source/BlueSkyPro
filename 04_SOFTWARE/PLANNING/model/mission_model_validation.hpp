#pragma once

#include "mission_model.hpp"

#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <vector>

namespace bluesky::planning {

inline bool isApprovedMissionTemplateId(const std::string &id)
{
    static const std::set<std::string> approved = {
        "MT-01", "MT-02", "MT-03", "MT-04", "MT-05", "MT-06", "MT-07",
        "MT-08", "MT-09", "MT-10", "MT-11", "MT-12", "MT-13"
    };
    return approved.count(id) != 0;
}

// This validates the manual mission model's structural completeness only.
// It does not establish airspace legality, route feasibility, or flight release.
inline std::vector<std::string> validateMissionModelStructure(const Mission &mission)
{
    std::vector<std::string> errors;
    if (mission.mission_mode != "AUTO" && mission.mission_mode != "MANUAL")
        errors.push_back("MISSION_MODE_INVALID");
    if ((!mission.lifecycle.mission_id.empty()
         && mission.lifecycle.mission_id != mission.identity.mission_id)
        || (!mission.lifecycle.mission_version.empty()
            && mission.lifecycle.mission_version != mission.identity.mission_version))
        errors.push_back("MISSION_LIFECYCLE_IDENTITY_MISMATCH");

    if (mission.mission_mode != "MANUAL")
        return errors;

    if (mission.selected_template_ids.empty())
        errors.push_back("MANUAL_TEMPLATE_REQUIRED");

    std::set<std::string> selectedTemplates;
    for (const auto &templateId : mission.selected_template_ids) {
        if (!isApprovedMissionTemplateId(templateId))
            errors.push_back("MISSION_TEMPLATE_UNSUPPORTED:" + templateId);
        if (!selectedTemplates.insert(templateId).second)
            errors.push_back("MISSION_TEMPLATE_DUPLICATE:" + templateId);
    }

    if (mission.geometry_objects.empty())
        errors.push_back("MANUAL_GEOMETRY_REQUIRED");

    std::set<std::string> geometryIds;
    std::set<std::string> associatedTemplates;
    for (const auto &geometry : mission.geometry_objects) {
        if (geometry.geometry_id.empty())
            errors.push_back("GEOMETRY_ID_REQUIRED");
        else if (!geometryIds.insert(geometry.geometry_id).second)
            errors.push_back("GEOMETRY_ID_DUPLICATE:" + geometry.geometry_id);

        if (geometry.coordinate_reference_system != "EPSG:4326")
            errors.push_back("GEOMETRY_CRS_UNSUPPORTED:" + geometry.geometry_id);

        const bool knownType =
            geometry.geometry_type == "POINT"
            || geometry.geometry_type == "WAYPOINT_ROUTE"
            || geometry.geometry_type == "POLYGON_AOI"
            || geometry.geometry_type == "COVERAGE_GRID"
            || geometry.geometry_type == "CORRIDOR"
            || geometry.geometry_type == "ORBIT"
            || geometry.geometry_type == "INSPECTION_POINTS"
            || geometry.geometry_type == "SEARCH_AREA"
            || geometry.geometry_type == "DELIVERY_ROUTE"
            || geometry.geometry_type == "OBSERVATION_POINTS";
        if (!knownType)
            errors.push_back("GEOMETRY_TYPE_UNSUPPORTED:" + geometry.geometry_id);

        std::size_t minimumCoordinates = 1;
        if (geometry.geometry_type == "WAYPOINT_ROUTE"
            || geometry.geometry_type == "CORRIDOR"
            || geometry.geometry_type == "DELIVERY_ROUTE")
            minimumCoordinates = 2;
        else if (geometry.geometry_type == "POLYGON_AOI"
                 || geometry.geometry_type == "COVERAGE_GRID"
                 || geometry.geometry_type == "SEARCH_AREA")
            minimumCoordinates = 4;

        if (geometry.coordinates.size() < minimumCoordinates)
            errors.push_back("GEOMETRY_COORDINATES_INCOMPLETE:" + geometry.geometry_id);

        for (const auto &point : geometry.coordinates) {
            if (!std::isfinite(point.longitude_deg) || point.longitude_deg < -180.0
                || point.longitude_deg > 180.0
                || !std::isfinite(point.latitude_deg) || point.latitude_deg < -90.0
                || point.latitude_deg > 90.0) {
                errors.push_back("GEOMETRY_COORDINATE_INVALID:" + geometry.geometry_id);
                break;
            }
        }

        if (geometry.template_ids.empty())
            errors.push_back("GEOMETRY_TEMPLATE_ASSOCIATION_REQUIRED:" + geometry.geometry_id);

        std::set<std::string> geometryTemplates;
        for (const auto &templateId : geometry.template_ids) {
            if (selectedTemplates.count(templateId) == 0)
                errors.push_back("GEOMETRY_TEMPLATE_NOT_SELECTED:" + geometry.geometry_id + ":" + templateId);
            if (!geometryTemplates.insert(templateId).second)
                errors.push_back("GEOMETRY_TEMPLATE_DUPLICATE:" + geometry.geometry_id + ":" + templateId);
            associatedTemplates.insert(templateId);
        }
    }

    for (const auto &templateId : selectedTemplates) {
        if (associatedTemplates.count(templateId) == 0)
            errors.push_back("TEMPLATE_GEOMETRY_REQUIRED:" + templateId);
    }

    // Operation estimates are optional in a draft. Once supplied, every
    // operation must resolve to selected task geometry and assigned equipment.
    std::set<std::string> operationIds;
    std::set<std::string> assignedEquipmentIds;
    std::set<std::string> assignedEquipmentProfiles;
    for (const auto &assignment : mission.equipment_assignments) {
        assignedEquipmentIds.insert(assignment.equipment_id);
        assignedEquipmentProfiles.insert(assignment.equipment_profile_id);
    }
    for (const auto &operation : mission.operations) {
        if (operation.operation_id.empty())
            errors.push_back("OPERATION_ID_REQUIRED");
        else if (!operationIds.insert(operation.operation_id).second)
            errors.push_back("OPERATION_ID_DUPLICATE:" + operation.operation_id);
        if (!isApprovedMissionTemplateId(operation.template_id)
            || selectedTemplates.count(operation.template_id) == 0)
            errors.push_back("OPERATION_TEMPLATE_NOT_SELECTED:" + operation.operation_id);
        if (geometryIds.count(operation.geometry_id) == 0)
            errors.push_back("OPERATION_GEOMETRY_NOT_FOUND:" + operation.operation_id);
        if (operation.operation_type.empty())
            errors.push_back("OPERATION_TYPE_REQUIRED:" + operation.operation_id);
        for (const auto &equipmentId : operation.equipment_ids) {
            if (assignedEquipmentIds.count(equipmentId) == 0)
                errors.push_back("OPERATION_EQUIPMENT_NOT_ASSIGNED:" + operation.operation_id + ":" + equipmentId);
        }

        const auto &estimate = operation.execution_estimate;
        const bool hasDuration = estimate.duration_s.has_value();
        const bool hasEnergy = estimate.energy_wh.has_value();
        if (estimate.state != "UNRESOLVED" && estimate.state != "VALIDATED")
            errors.push_back("OPERATION_ESTIMATE_STATE_INVALID:" + operation.operation_id);
        if (hasDuration != hasEnergy
            || (hasDuration && (!std::isfinite(*estimate.duration_s) || *estimate.duration_s < 0.0
                                || !std::isfinite(*estimate.energy_wh) || *estimate.energy_wh < 0.0)))
            errors.push_back("OPERATION_ESTIMATE_VALUES_INVALID:" + operation.operation_id);
        if (estimate.state == "UNRESOLVED") {
            if (hasDuration || hasEnergy || !estimate.source_id.empty()
                || !estimate.source_version.empty() || !estimate.method_version.empty())
                errors.push_back("UNRESOLVED_OPERATION_ESTIMATE_HAS_VALUES:" + operation.operation_id);
        } else {
            if (!hasDuration || estimate.source_id.empty() || estimate.source_version.empty()
                || estimate.method_version.empty() || estimate.vehicle_id.empty()
                || estimate.vehicle_configuration_version.empty())
                errors.push_back("VALIDATED_OPERATION_ESTIMATE_PROVENANCE_REQUIRED:" + operation.operation_id);
            const auto vehicle = std::find_if(mission.vehicle_assignments.begin(), mission.vehicle_assignments.end(),
                [&](const VehicleAssignment &assignment) {
                    return assignment.vehicle_id == estimate.vehicle_id
                        && assignment.configuration_version == estimate.vehicle_configuration_version;
                });
            if (vehicle == mission.vehicle_assignments.end())
                errors.push_back("OPERATION_ESTIMATE_VEHICLE_CONFIGURATION_MISMATCH:" + operation.operation_id);
            if (estimate.source_type != "VEHICLE_MODEL" && estimate.source_type != "EQUIPMENT_MODEL"
                && estimate.source_type != "MEASURED_PROFILE" && estimate.source_type != "OPERATOR_APPROVED")
                errors.push_back("OPERATION_ESTIMATE_SOURCE_TYPE_INVALID:" + operation.operation_id);
            for (const auto &profileId : estimate.equipment_profile_ids) {
                if (assignedEquipmentProfiles.count(profileId) == 0)
                    errors.push_back("OPERATION_ESTIMATE_EQUIPMENT_PROFILE_NOT_ASSIGNED:" + operation.operation_id + ":" + profileId);
            }
            if (estimate.source_type == "EQUIPMENT_MODEL" && estimate.equipment_profile_ids.empty())
                errors.push_back("OPERATION_ESTIMATE_EQUIPMENT_PROFILE_REQUIRED:" + operation.operation_id);
        }
    }

    return errors;
}

} // namespace bluesky::planning
