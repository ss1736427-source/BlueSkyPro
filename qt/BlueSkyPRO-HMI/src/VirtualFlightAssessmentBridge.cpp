#include "VirtualFlightAssessmentBridge.h"
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
