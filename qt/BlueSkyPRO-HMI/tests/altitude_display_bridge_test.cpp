#include "AltitudeDisplayBridge.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <QCoreApplication>
#include <cassert>
#include <iostream>

using namespace bluesky::operations;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    AltitudeDisplayBridge bridge;

    assert(!bridge.available());
    assert(bridge.statusText() == QStringLiteral("UNAVAILABLE"));

    MavlinkAltitudeDisplayAdapterResult result;
    result.decision = {AltitudeDisplaySource::Unavailable, 0.0, false, false};
    assert(!bridge.publishAdapterResult(result));
    assert(!bridge.available());

    result.decision = {AltitudeDisplaySource::RadioAltimeterAgl, 99.5, true, false};
    assert(bridge.publishAdapterResult(result));
    assert(bridge.available());
    assert(!bridge.degraded());
    assert(bridge.valueText() == QStringLiteral("99.5 m"));
    assert(bridge.sourceText() == QStringLiteral("RA · AGL"));
    assert(bridge.statusText() == QStringLiteral("LIVE"));

    result.decision = {AltitudeDisplaySource::BarometricStandardPressure, 123.4, true, false};
    result.timestamp_quality_degraded = true;
    assert(bridge.publishAdapterResult(result));
    assert(bridge.available());
    assert(bridge.degraded());
    assert(bridge.sourceText() == QStringLiteral("BARO · STD 1013.25 hPa"));
    assert(bridge.statusText() == QStringLiteral("DEGRADED"));

    result.decision = {AltitudeDisplaySource::BarometricStandardPressureDegraded, 123.4, true, true};
    result.timestamp_quality_degraded = false;
    assert(bridge.publishAdapterResult(result));
    assert(bridge.degraded());

    result.decision = {AltitudeDisplaySource::RadioAltimeterAgl, -1.0, true, false};
    assert(!bridge.publishAdapterResult(result));
    assert(!bridge.available());
    assert(bridge.statusText() == QStringLiteral("UNAVAILABLE"));

    bridge.markUnavailable();
    assert(!bridge.available());
    assert(!bridge.degraded());
    assert(bridge.valueText() == QStringLiteral("—"));
    assert(bridge.sourceText() == QStringLiteral("ALTITUDE"));
    assert(bridge.statusText() == QStringLiteral("UNAVAILABLE"));

    std::cout << "altitude_display_bridge_test: PASS\n";
    return 0;
}
