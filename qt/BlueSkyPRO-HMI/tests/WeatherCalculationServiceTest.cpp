#include "WeatherCalculationService.h"

#include <QCoreApplication>
#include <cassert>

int main(int argc,char** argv)
{
    QCoreApplication app(argc,argv);
    WeatherCalculationService service;
    const bool candidatesAccepted = service.evaluateCandidates(
        {}, {}, QStringLiteral("{}"), QString(), {}, {QStringLiteral("time")});
    assert(!candidatesAccepted);
    if (!service.decoderAvailable())
        assert(service.lastError() == QStringLiteral("GRIB2_DECODER_NOT_AVAILABLE"));
    else
        assert(service.lastError() == QStringLiteral("WEATHER_CANDIDATE_SET_REQUIRED"));

    const bool unsupportedObjectiveAccepted = service.evaluateCandidates(
        {}, {QVariantMap{}}, QStringLiteral("{}"), QString(), {},
        {QStringLiteral("minimum_cost")});
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
