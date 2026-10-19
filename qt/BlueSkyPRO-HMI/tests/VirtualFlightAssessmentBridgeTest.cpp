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
 return 0;
}
