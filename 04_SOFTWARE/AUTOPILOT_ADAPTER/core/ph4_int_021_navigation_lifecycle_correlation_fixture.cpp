#include "navigation_adapter_baseline.hpp"

#include <iostream>
#include <string>

int main() {
    using namespace bluesky::navigation;

    NavigationAdapterBaseline adapter("SRC-06");

    const auto rejected_source = adapter.acceptSample(NavigationSample{
        .sourceId = "SRC-WRONG",
        .timestampUtc = "2026-09-15T12:00:00Z",
        .sourceTimestampMs = 1000,
        .adapterTimestampMs = 1001,
        .valid = true,
        .stale = false,
    });

    NavigationSample valid{};
    valid.sourceId = "SRC-06";
    valid.timestampUtc = "2026-09-15T12:00:01Z";
    valid.sourceTimestampMs = 2000;
    valid.adapterTimestampMs = 2001;
    valid.position = {60.1699, 24.9384, 80.0};
    valid.velocity = {12.0, 3.0, 0.0};
    valid.attitude = {1.0, -2.0, 90.0};
    valid.fixType = FixType::RTKFixed;
    valid.quality = NavigationQuality::Usable;
    valid.correctionState = CorrectionState::Healthy;
    valid.satellites = 18;
    valid.horizontalAccuracyM = 0.03;
    valid.verticalAccuracyM = 0.05;
    valid.valid = true;
    valid.stale = false;

    const auto accepted = adapter.acceptSample(valid);
    const auto readback = adapter.readSample();

    NavigationSample stale = valid;
    stale.sourceTimestampMs = 3000;
    stale.timestampUtc = "2026-09-15T12:00:02Z";
    stale.stale = true;
    const auto stale_result = adapter.acceptSample(stale);

    NavigationSample invalid = valid;
    invalid.sourceTimestampMs = 4000;
    invalid.timestampUtc = "2026-09-15T12:00:03Z";
    invalid.valid = false;
    const auto invalid_result = adapter.acceptSample(invalid);

    const bool readback_match = readback.has_value()
        && readback->sourceId == valid.sourceId
        && readback->sourceTimestampMs == valid.sourceTimestampMs
        && readback->position.latitudeDeg == valid.position.latitudeDeg
        && readback->position.longitudeDeg == valid.position.longitudeDeg
        && readback->position.altitudeM == valid.position.altitudeM
        && readback->fixType == valid.fixType
        && readback->quality == valid.quality
        && readback->correctionState == valid.correctionState
        && readback->satellites == valid.satellites;

    std::cout << "PH4_INT_021"
              << "|FLIGHT-RECORD-PH4-INT-021"
              << "|UAV-NAV-021"
              << "|GNSS_RTK_NTRIP"
              << "|BASELINE-1"
              << "|SOURCE_REJECTED=" << (!rejected_source.accepted ? "PASS" : "FAIL")
              << "|VALID_ACCEPTED=" << (accepted.accepted ? "PASS" : "FAIL")
              << "|READBACK=" << (readback.has_value() ? "PASS" : "FAIL")
              << "|CORRELATION_MATCH=" << (readback_match ? "PASS" : "FAIL")
              << "|STALE_REJECTED=" << (!stale_result.accepted ? "PASS" : "FAIL")
              << "|INVALID_REJECTED=" << (!invalid_result.accepted ? "PASS" : "FAIL")
              << "|QUALITY=" << (adapter.getQuality() == NavigationQuality::Usable ? "USABLE" : "FAIL")
              << "|CORRECTION=" << (adapter.getCorrectionState() == CorrectionState::Healthy ? "HEALTHY" : "FAIL")
              << "|SRC_TS=" << valid.sourceTimestampMs
              << "|ADAPTER_TS=" << valid.adapterTimestampMs
              << '\n';

    return rejected_source.accepted || !accepted.accepted || !readback_match
        || stale_result.accepted || invalid_result.accepted ? 1 : 0;
}
