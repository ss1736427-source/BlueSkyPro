#include "WeatherCalculationService.h"

#include <QCoreApplication>
#include <cassert>

static QVariantMap validNotamSnapshot()
{
    return {
        {QStringLiteral("valid"), true},
        {QStringLiteral("snapshotId"), QStringLiteral("TEST-NOTAM-SNAPSHOT")},
        {QStringLiteral("sourceId"), QStringLiteral("TEST-SOURCE")},
        {QStringLiteral("sourceVersion"), QStringLiteral("1")},
        {QStringLiteral("evaluationEpochS"), 1800000000LL},
        {QStringLiteral("coverageCenterLatitudeDeg"), 0.0},
        {QStringLiteral("coverageCenterLongitudeDeg"), 0.0},
        {QStringLiteral("coverageRadiusM"), 1000.0},
        {QStringLiteral("plannedMissionStartEpochS"), 1800000000LL},
        {QStringLiteral("plannedMissionEndEpochS"), 1800003600LL},
        {QStringLiteral("safetyBufferM"), 25.0},
        {QStringLiteral("zones"), QVariantList{}}
    };
}

int main(int argc,char** argv)
{
    QCoreApplication app(argc,argv);
    WeatherCalculationService service;
    const bool missingNotamAccepted = service.evaluateCandidates(
        {}, {}, QStringLiteral("{}"), QString(), {}, {QStringLiteral("time")}, {});
    assert(!missingNotamAccepted);
    assert(service.lastError() == QStringLiteral("NOTAM_SNAPSHOT_REQUIRED_FOR_WEATHER_RERANKING"));

    const bool candidatesAccepted = service.evaluateCandidates(
        {}, {}, QStringLiteral("{}"), QString(), {}, {QStringLiteral("time")}, validNotamSnapshot());
    assert(!candidatesAccepted);
    if (!service.decoderAvailable())
        assert(service.lastError() == QStringLiteral("GRIB2_DECODER_NOT_AVAILABLE"));
    else
        assert(service.lastError() == QStringLiteral("WEATHER_CANDIDATE_SET_REQUIRED"));

    const bool unsupportedObjectiveAccepted = service.evaluateCandidates(
        {}, {QVariantMap{}}, QStringLiteral("{}"), QString(), {},
        {QStringLiteral("minimum_cost")}, validNotamSnapshot());
    assert(!unsupportedObjectiveAccepted);
    if (service.decoderAvailable())
        assert(service.lastError()
               == QStringLiteral("WEATHER_RERANKING_REQUIRES_TIME_ENERGY_OR_RESERVE_OBJECTIVE"));
    else
        assert(service.lastError() == QStringLiteral("GRIB2_DECODER_NOT_AVAILABLE"));

    const bool accepted=service.evaluateRoute(
        {},{},QStringLiteral("{}"),QString(),{});
    assert(!accepted);
    assert(service.state()==QStringLiteral("BLOCKED"));
    if (!service.decoderAvailable())
        assert(service.lastError()==QStringLiteral("GRIB2_DECODER_NOT_AVAILABLE"));
    else
        assert(service.lastError()==QStringLiteral("DOWNLOADED_GRIB2_PRODUCTS_REQUIRED"));
    return 0;
}
