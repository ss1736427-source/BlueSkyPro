#include "VirtualFlightAssessmentBridge.h"
#include <QCoreApplication>
#include <cassert>
int main(int argc,char **argv) {
 QCoreApplication app(argc,argv); VirtualFlightAssessmentBridge b;
 const auto calm=b.assessSyntheticFixture(0);
 assert(calm.value("assessed").toBool()); assert(calm.value("candidateStructurallyValid").toBool());
 assert(calm.value("airspaceClear").toBool()); assert(calm.value("notamClear").toBool());
 assert(calm.value("combinedClear").toBool()); assert(!calm.value("flightReleaseEligible").toBool());
 const auto blocked=b.assessSyntheticFixture(1);
 assert(blocked.value("assessed").toBool()); assert(!blocked.value("notamClear").toBool());
 assert(!blocked.value("combinedClear").toBool()); assert(!blocked.value("flightReleaseEligible").toBool());
 const auto lowVis=b.assessSyntheticFixture(2);
 assert(lowVis.value("assessed").toBool()); assert(lowVis.value("combinedClear").toBool());
 assert(!lowVis.value("flightReleaseEligible").toBool());
 assert(lowVis.value("scope").toString().contains("Weather thresholds"));
 const auto invalid=b.assessSyntheticFixture(99);
 assert(!invalid.value("assessed").toBool()); assert(!invalid.value("flightReleaseEligible").toBool());
 const QString manualGeometry = QStringLiteral(
     "{\"origin\":{\"longitudeDeg\":30.0,\"latitudeDeg\":59.0,\"coordinateReferenceSystem\":\"EPSG:4326\",\"originKind\":\"PLANNED_TAKEOFF\"},\"objects\":[{"
     "\"geometryId\":\"GEO-MT-05-TEST\","
     "\"geometryType\":\"CORRIDOR\","
     "\"coordinateReferenceSystem\":\"EPSG:4326\","
     "\"coordinates\":[[30.0,59.0,40.0,\"AGL\"],[30.01,59.01,45.0,\"AGL\"]],"
     "\"templateIds\":[\"MT-05\"],\"version\":1}]}");
 const auto manualClear = b.assessManualGeometry(manualGeometry,
     {QStringLiteral("MT-05")}, QStringLiteral("VF-MANUAL-TEST"), 1, true, 0);
 assert(manualClear.value("assessed").toBool());
 assert(manualClear.value("candidateStructurallyValid").toBool());
 assert(manualClear.value("combinedClear").toBool());
 assert(manualClear.value("routeSource").toString() == QStringLiteral("OPERATOR_MANUAL_GEOMETRY"));
 assert(!manualClear.value("flightReleaseEligible").toBool());
 const auto manualBlocked = b.assessManualGeometry(manualGeometry,
     {QStringLiteral("MT-05")}, QStringLiteral("VF-MANUAL-TEST"), 1, true, 1);
 assert(manualBlocked.value("assessed").toBool());
 assert(!manualBlocked.value("notamClear").toBool());
 assert(!manualBlocked.value("combinedClear").toBool());
 assert(!manualBlocked.value("flightReleaseEligible").toBool());
 const QString missingDatum = QStringLiteral(
     "{\"origin\":{\"longitudeDeg\":30.0,\"latitudeDeg\":59.0,\"coordinateReferenceSystem\":\"EPSG:4326\",\"originKind\":\"PLANNED_TAKEOFF\"},\"objects\":[{"
     "\"geometryId\":\"GEO-MT-05-NO-DATUM\","
     "\"geometryType\":\"CORRIDOR\","
     "\"coordinateReferenceSystem\":\"EPSG:4326\","
     "\"coordinates\":[[30.0,59.0,40.0],[30.01,59.01,45.0]],"
     "\"templateIds\":[\"MT-05\"],\"version\":1}]}");
 const auto noDatum = b.assessManualGeometry(missingDatum,
     {QStringLiteral("MT-05")}, QStringLiteral("VF-MANUAL-TEST"), 1, true, 0);
 assert(!noDatum.value("assessed").toBool());
 assert(!noDatum.value("combinedClear").toBool());
 assert(!noDatum.value("flightReleaseEligible").toBool());
 return 0;
}
