#include "VirtualFlightAssessmentBridge.h"
#include "ManualMissionModelAdapter.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "04_SOFTWARE/PLANNING/manual_route_candidate_assessment.hpp"
#include <QStringList>
#include <string>
using namespace bluesky::planning;
namespace {
constexpr std::int64_t kSimulationEpochSeconds = 1800000000;
Mission syntheticMission() {
    Mission m; m.identity.mission_id="VF-SYNTHETIC-MISSION-001";
    m.identity.mission_version="1"; m.mission_mode="MANUAL";
    m.selected_template_ids={"VF-TEST-LINE"}; return m;
}
MissionGeometryObject syntheticGeometry() {
    MissionGeometryObject g; g.geometry_id="VF-SYNTHETIC-ROUTE-001";
    g.geometry_type="WAYPOINT_ROUTE"; g.coordinate_reference_system="EPSG:4326";
    g.template_ids={"VF-TEST-LINE"}; g.version=1;
    // Fixed synthetic coordinates only; not an operating location or map route.
    g.coordinates={{30.0000,59.0000,40.0,std::string("AGL")},
                   {30.0100,59.0050,40.0,std::string("AGL")},
                   {30.0200,59.0100,40.0,std::string("AGL")}};
    return g;
}
ManualRouteBuildRequirements syntheticRequirements() {
    return {"virtual-flight-synthetic-route-generator","1.0","VF-FIXTURES-1.0",
      "VF-TERRAIN-SYNTHETIC-1","VF-AIRSPACE-SYNTHETIC-1",
      "VF-WEATHER-SYNTHETIC-1","VF-WIND-SYNTHETIC-1",20.0,80.0,10.0,8.0};
}
NotamZone syntheticZone(const std::string &id,const GeoPoint &center,double radius) {
    NotamZone z; z.notam_id=id; z.restriction_type=NotamRestrictionType::Prohibited;
    z.geometry_type=NotamGeometryType::Circle; z.center=center; z.radius_m=radius;
    z.lower_altitude_m=0.0; z.upper_altitude_m=100.0;
    z.valid_from_epoch_s=1700000000; z.valid_to_epoch_s=1900000000;
    z.altitude_reference=NotamAltitudeReference::AGL; return z;
}
QStringList qstrings(const std::vector<std::string> &values) {
    QStringList out; for (const auto &v:values) out.push_back(QString::fromStdString(v)); return out;
}
}
QVariantMap VirtualFlightAssessmentBridge::assessSyntheticFixture(int scenarioIndex) const {
    QVariantMap out;
    if (scenarioIndex<0 || scenarioIndex>2) {
        out.insert("assessed",false); out.insert("status","BLOCKED");
        out.insert("errors",QStringList{"SYNTHETIC_PROFILE_INDEX_INVALID"});
        out.insert("flightReleaseEligible",false); return out;
    }
    const auto candidate=buildManualRouteCandidate(syntheticMission(),syntheticGeometry(),syntheticRequirements());
    ConstrainedEnvironmentSnapshot airspace;
    airspace.snapshot_id="VF-AIRSPACE-SYNTHETIC-1";
    airspace.snapshot_version="VF-AIRSPACE-SYNTHETIC-1.0";
    airspace.calculation_input_version="VF-FIXTURES-1.0"; airspace.complete=true;
    NotamSnapshot notams; notams.snapshot_id=airspace.snapshot_id;
    notams.source_id="VF-SYNTHETIC-NOTAM-FIXTURE";
    notams.source_version="VF-NOTAM-FIXTURES-1.0";
    notams.evaluation_epoch_s=kSimulationEpochSeconds;
    const GeoPoint onRoute{59.0050,30.0100}, away{59.0200,30.0500};
    const int count=scenarioIndex==0?2:(scenarioIndex==1?4:3);
    for(int i=0;i<count;++i) {
        const bool intersects=(scenarioIndex==1 && i==0);
        notams.zones.push_back(syntheticZone(
          "VF-NOTAM-SYN-"+std::to_string(scenarioIndex+1)+"-"+std::to_string(i+1),
          intersects?onRoute:away,intersects?300.0:100.0));
    }
    const auto a=assessManualRouteCandidateAirspaceAndNotams(candidate,airspace,notams,"VF-FIXTURES-1.0");
    out.insert("assessed",a.assessed); out.insert("candidateStructurallyValid",a.candidate_structurally_valid);
    out.insert("airspaceClear",a.airspace_clear); out.insert("notamClear",a.notam_clear);
    out.insert("combinedClear",a.combined_clear); out.insert("flightReleaseEligible",false);
    out.insert("profileId",QString("VF-WX-SYN-00%1").arg(scenarioIndex+1));
    out.insert("datasetVersion","VF-FIXTURES-1.0"); out.insert("routeSource","FIXED_SYNTHETIC_TEST_ROUTE");
    out.insert("notamSource","VF-SYNTHETIC-NOTAM-FIXTURE"); out.insert("notamRecordCount",count);
    out.insert("evaluationEpochSeconds",QVariant::fromValue<qlonglong>(kSimulationEpochSeconds));
    out.insert("status",a.assessed && a.combined_clear ? "ASSESSMENT_CLEAR_NON_OPERATIONAL" : "BLOCKED_OR_NOT_ASSESSED");
    out.insert("errors",qstrings(a.errors));
    out.insert("scope","Airspace + NOTAM only. Weather thresholds, terrain, obstacles, energy, authorizations and flight release are not assessed.");
    return out;
}

QVariantMap VirtualFlightAssessmentBridge::assessManualGeometry(
    const QString &geometryJson,
    const QStringList &selectedTemplateIds,
    const QString &missionId,
    int missionVersion,
    bool compositionComplete,
    int scenarioIndex) const
{
    QVariantMap out;
    out.insert("flightReleaseEligible", false);
    out.insert("routeSource", "OPERATOR_MANUAL_GEOMETRY");
    out.insert("datasetVersion", "VF-FIXTURES-1.0");
    out.insert("notamSource", "VF-SYNTHETIC-NOTAM-FIXTURE");
    out.insert("scope", "Operator geometry assessed against synthetic airspace/NOTAM only. Weather thresholds, terrain, obstacles, energy, authorizations and flight release are not assessed.");
    if (scenarioIndex < 0 || scenarioIndex > 2) {
        out.insert("assessed", false);
        out.insert("status", "BLOCKED");
        out.insert("errors", QStringList{"SYNTHETIC_PROFILE_INDEX_INVALID"});
        return out;
    }
    ManualMissionModelAdapter adapter;
    const QVariantMap modelResult = adapter.buildMission(
        geometryJson, selectedTemplateIds, missionId, missionVersion, compositionComplete);
    if (!modelResult.value("valid").toBool()) {
        out.insert("assessed", false);
        out.insert("candidateStructurallyValid", false);
        out.insert("combinedClear", false);
        out.insert("status", "BLOCKED_INVALID_MANUAL_GEOMETRY");
        out.insert("errors", modelResult.value("errors").toStringList());
        out.insert("notamRecordCount", 0);
        return out;
    }

    const QJsonDocument modelDoc = QJsonDocument::fromJson(
        modelResult.value("missionModelJson").toString().toUtf8());
    const QJsonObject modelObject = modelDoc.object();
    Mission mission;
    mission.identity.mission_id = modelObject.value("missionId").toString().toStdString();
    mission.identity.mission_version = std::to_string(modelObject.value("missionVersion").toInt());
    mission.mission_mode = "MANUAL";
    const QJsonObject originObject = modelObject.value("origin").toObject();
    if (!originObject.isEmpty()) {
        MissionOrigin origin;
        origin.longitude_deg = originObject.value("longitudeDeg").toDouble();
        origin.latitude_deg = originObject.value("latitudeDeg").toDouble();
        origin.coordinate_reference_system =
            originObject.value("coordinateReferenceSystem").toString().toStdString();
        origin.origin_kind = originObject.value("originKind").toString().toStdString();
        if (originObject.value("horizontalAccuracyM").isDouble())
            origin.horizontal_accuracy_m = originObject.value("horizontalAccuracyM").toDouble();
        if (originObject.value("trueNorthBearingDeg").isDouble())
            origin.true_north_bearing_deg = originObject.value("trueNorthBearingDeg").toDouble();
        mission.origin = origin;
    }
    for (const auto &id : selectedTemplateIds)
        mission.selected_template_ids.push_back(id.toStdString());

    const QJsonArray objects = modelObject.value("geometryObjects").toArray();
    if (objects.size() != 1) {
        out.insert("assessed", false);
        out.insert("candidateStructurallyValid", false);
        out.insert("combinedClear", false);
        out.insert("status", "BLOCKED_SINGLE_LINE_GEOMETRY_REQUIRED");
        out.insert("errors", QStringList{"VF_ASSESSMENT_REQUIRES_EXACTLY_ONE_LINE_GEOMETRY_OBJECT"});
        out.insert("notamRecordCount", 0);
        return out;
    }
    const QJsonObject sourceGeometry = objects.at(0).toObject();
    MissionGeometryObject geometry;
    geometry.geometry_id = sourceGeometry.value("geometryId").toString().toStdString();
    geometry.geometry_type = sourceGeometry.value("geometryType").toString().toStdString();
    geometry.coordinate_reference_system = sourceGeometry.value("coordinateReferenceSystem").toString().toStdString();
    geometry.version = sourceGeometry.value("version").toInt(1);
    for (const auto &id : sourceGeometry.value("templateIds").toArray())
        geometry.template_ids.push_back(id.toString().toStdString());
    for (const auto &coordinateValue : sourceGeometry.value("coordinates").toArray()) {
        const QJsonArray coordinate = coordinateValue.toArray();
        if (coordinate.size() < 2) {
            out.insert("assessed", false);
            out.insert("candidateStructurallyValid", false);
            out.insert("combinedClear", false);
            out.insert("status", "BLOCKED_INVALID_COORDINATE");
            out.insert("errors", QStringList{"GEOMETRY_COORDINATE_INVALID"});
            out.insert("notamRecordCount", 0);
            return out;
        }
        MissionGeometryVertex vertex;
        vertex.longitude_deg = coordinate.at(0).toDouble();
        vertex.latitude_deg = coordinate.at(1).toDouble();
        if (coordinate.size() > 2 && coordinate.at(2).isDouble())
            vertex.altitude_m = coordinate.at(2).toDouble();
        if (coordinate.size() > 3 && coordinate.at(3).isString())
            vertex.altitude_reference = coordinate.at(3).toString().toStdString();
        geometry.coordinates.push_back(vertex);
    }

    const auto candidate = buildManualRouteCandidate(
        mission, geometry, syntheticRequirements());
    if (!candidate.valid) {
        out.insert("assessed", false);
        out.insert("candidateStructurallyValid", false);
        out.insert("combinedClear", false);
        out.insert("status", "BLOCKED_ROUTE_CANDIDATE_INVALID");
        out.insert("errors", qstrings(candidate.errors));
        out.insert("notamRecordCount", 0);
        return out;
    }

    ConstrainedEnvironmentSnapshot airspace;
    airspace.snapshot_id = "VF-AIRSPACE-SYNTHETIC-1";
    airspace.snapshot_version = "VF-AIRSPACE-SYNTHETIC-1.0";
    airspace.calculation_input_version = "VF-FIXTURES-1.0";
    airspace.complete = true;
    NotamSnapshot notams;
    notams.snapshot_id = airspace.snapshot_id;
    notams.source_id = "VF-SYNTHETIC-NOTAM-FIXTURE";
    notams.source_version = "VF-NOTAM-FIXTURES-1.0";
    notams.evaluation_epoch_s = kSimulationEpochSeconds;
    const auto &waypoints = candidate.route.waypoints;
    const GeoPoint routeCenter = waypoints[waypoints.size() / 2].position;
    const GeoPoint away{routeCenter.latitude_deg + 2.0, routeCenter.longitude_deg + 2.0};
    const int count = scenarioIndex == 0 ? 2 : (scenarioIndex == 1 ? 4 : 3);
    for (int i = 0; i < count; ++i) {
        const bool intersects = scenarioIndex == 1 && i == 0;
        notams.zones.push_back(syntheticZone(
            "VF-NOTAM-MANUAL-" + std::to_string(scenarioIndex + 1) + "-" + std::to_string(i + 1),
            intersects ? routeCenter : away, intersects ? 300.0 : 100.0));
    }
    const auto assessment = assessManualRouteCandidateAirspaceAndNotams(
        candidate, airspace, notams, "VF-FIXTURES-1.0");
    out.insert("assessed", assessment.assessed);
    out.insert("candidateStructurallyValid", assessment.candidate_structurally_valid);
    out.insert("airspaceClear", assessment.airspace_clear);
    out.insert("notamClear", assessment.notam_clear);
    out.insert("combinedClear", assessment.combined_clear);
    out.insert("profileId", QString("VF-WX-SYN-00%1").arg(scenarioIndex + 1));
    out.insert("notamRecordCount", count);
    out.insert("evaluationEpochSeconds", QVariant::fromValue<qlonglong>(kSimulationEpochSeconds));
    out.insert("status", assessment.assessed && assessment.combined_clear
                             ? "ASSESSMENT_CLEAR_NON_OPERATIONAL" : "BLOCKED_OR_NOT_ASSESSED");
    out.insert("errors", qstrings(assessment.errors));
    return out;
}
