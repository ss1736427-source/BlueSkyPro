#include "altitude_display_source_policy.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using namespace bluesky::operations;

namespace {

AltitudeReading reading(double value, const char* source) {
    return {value, true, true, source, 1000, 1000};
}

void testRadioAltimeterAtAndBelow100m() {
    const auto baro = reading(340.0, "baro-std");
    auto radio = reading(0.0, "radio-altimeter");
    auto decision = select_altitude_display(radio, baro);
    assert(decision.source == AltitudeDisplaySource::RadioAltimeterAgl);
    assert(decision.value_m == 0.0);

    radio.value_m = 99.9;
    decision = select_altitude_display(radio, baro);
    assert(decision.source == AltitudeDisplaySource::RadioAltimeterAgl);
    assert(decision.value_m == 99.9);

    radio.value_m = 100.0;
    decision = select_altitude_display(radio, baro);
    assert(decision.source == AltitudeDisplaySource::RadioAltimeterAgl);
    assert(decision.value_m == 100.0);
}

void testBarometricAltitudeAbove100mRadioReading() {
    const auto radio = reading(100.1, "radio-altimeter");
    const auto baro = reading(420.0, "baro-std");
    const auto decision = select_altitude_display(radio, baro);
    assert(decision.source == AltitudeDisplaySource::BarometricStandardPressure);
    assert(decision.value_available);
    assert(!decision.degraded);
    assert(decision.value_m == 420.0);
}

void testMissingRadioAltimeterUsesOnlyDegradedHighBaroFallback() {
    AltitudeReading radio;
    const auto highBaro = reading(150.0, "baro-std");
    auto decision = select_altitude_display(radio, highBaro);
    assert(decision.source ==
           AltitudeDisplaySource::BarometricStandardPressureDegraded);
    assert(decision.value_available);
    assert(decision.degraded);

    const auto lowBaro = reading(100.0, "baro-std");
    decision = select_altitude_display(radio, lowBaro);
    assert(decision.source == AltitudeDisplaySource::Unavailable);
    assert(!decision.value_available);
}

void testInvalidOrStaleRadioReadingDoesNotSelectRadioAltitude() {
    auto radio = reading(20.0, "radio-altimeter");
    radio.fresh = false;
    const auto baro = reading(200.0, "baro-std");
    const auto decision = select_altitude_display(radio, baro);
    assert(decision.source ==
           AltitudeDisplaySource::BarometricStandardPressureDegraded);
    assert(decision.degraded);
}

void testMissingBarometerBlocksCrossover() {
    const auto radio = reading(101.0, "radio-altimeter");
    AltitudeReading baro;
    const auto decision = select_altitude_display(radio, baro);
    assert(decision.source == AltitudeDisplaySource::Unavailable);
    assert(!decision.value_available);
}

void testNonFiniteAndNegativeRadioReadingsAreRejected() {
    auto radio = reading(std::numeric_limits<double>::quiet_NaN(), "radio-altimeter");
    const auto baro = reading(300.0, "baro-std");
    auto decision = select_altitude_display(radio, baro);
    assert(decision.source ==
           AltitudeDisplaySource::BarometricStandardPressureDegraded);

    radio.value_m = -1.0;
    decision = select_altitude_display(radio, baro);
    assert(decision.source ==
           AltitudeDisplaySource::BarometricStandardPressureDegraded);
}

void testMalformedProvenanceAndTimestampsAreRejected() {
    auto radio = reading(10.0, "radio-altimeter");
    radio.source_id.clear();
    auto baro = reading(80.0, "baro-std");
    auto decision = select_altitude_display(radio, baro);
    assert(decision.source == AltitudeDisplaySource::Unavailable);

    radio = reading(10.0, "radio-altimeter");
    radio.source_timestamp_ms = 2000;
    radio.received_timestamp_ms = 1000;
    decision = select_altitude_display(radio, baro);
    assert(decision.source == AltitudeDisplaySource::Unavailable);
}

} // namespace

int main() {
    testRadioAltimeterAtAndBelow100m();
    testBarometricAltitudeAbove100mRadioReading();
    testMissingRadioAltimeterUsesOnlyDegradedHighBaroFallback();
    testInvalidOrStaleRadioReadingDoesNotSelectRadioAltitude();
    testMissingBarometerBlocksCrossover();
    testNonFiniteAndNegativeRadioReadingsAreRejected();
    testMalformedProvenanceAndTimestampsAreRejected();
    std::cout << "altitude_display_source_policy_test: PASS\n";
    return 0;
}
