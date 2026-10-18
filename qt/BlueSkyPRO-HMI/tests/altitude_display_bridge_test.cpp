#include "AltitudeDisplayBridge.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <QCoreApplication>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    AltitudeDisplayBridge bridge;

    assert(!bridge.available());
    assert(!bridge.degraded());
    assert(bridge.valueText() == QStringLiteral("—"));
    assert(bridge.statusText() == QStringLiteral("UNAVAILABLE"));

    assert(!bridge.publishDecision(std::numeric_limits<double>::quiet_NaN(),
                                   QStringLiteral("RA_AGL"), false, true));
    assert(!bridge.available());
    assert(bridge.statusText() == QStringLiteral("UNAVAILABLE"));

    assert(!bridge.publishDecision(-1.0, QStringLiteral("RA_AGL"), false, true));
    assert(!bridge.publishDecision(42.0, QStringLiteral("GPS_ALT"), false, true));
    assert(!bridge.available());

    assert(bridge.publishDecision(99.5, QStringLiteral("RA_AGL"), false, true));
    assert(bridge.available());
    assert(!bridge.degraded());
    assert(bridge.valueText() == QStringLiteral("99.5 m"));
    assert(bridge.sourceText() == QStringLiteral("RA · AGL"));
    assert(bridge.statusText() == QStringLiteral("LIVE"));

    assert(bridge.publishDecision(123.4, QStringLiteral("BARO_STD"), true, true));
    assert(bridge.available());
    assert(bridge.degraded());
    assert(bridge.sourceText() == QStringLiteral("BARO · STD 1013.25 hPa"));
    assert(bridge.statusText() == QStringLiteral("DEGRADED"));

    assert(!bridge.publishDecision(123.4, QStringLiteral("BARO_STD"), false, false));
    assert(!bridge.available());
    assert(bridge.statusText() == QStringLiteral("UNAVAILABLE"));

    assert(bridge.publishDecision(123.4, QStringLiteral("BARO_STD"), false, true));
    assert(bridge.available());
    assert(!bridge.degraded());

    bridge.markUnavailable();
    assert(!bridge.available());
    assert(!bridge.degraded());
    assert(bridge.valueText() == QStringLiteral("—"));
    assert(bridge.sourceText() == QStringLiteral("ALTITUDE"));
    assert(bridge.statusText() == QStringLiteral("UNAVAILABLE"));

    std::cout << "altitude_display_bridge_test: PASS\n";
    return 0;
}
