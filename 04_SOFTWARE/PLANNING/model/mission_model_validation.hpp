#pragma once

#include "mission_model.hpp"

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

    return errors;
}

} // namespace bluesky::planning
