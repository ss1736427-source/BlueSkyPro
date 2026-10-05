#include "bluesky_event_hmi_contract.hpp"

#include <iostream>

using namespace bluesky::hmi;

static bool check(bool condition, const char* label) {
    if (!condition) { std::cerr << "FAIL: " << label << '\n'; return false; }
    std::cout << "PASS: " << label << '\n';
    return true;
}

int main() {
    try {
        BlueSkyEvent event{
            "EV-15-001", "2026-09-14T17:00:00Z", "SRC-14",
            "c2.link_state", "DEGRADED", EventSeverity::WARNING, EventState::ACTIVE
        };
        BlueSkyEventHmiContract::validate(event);
        if (!check(event.event_id == "EV-15-001", "event_id")) return 1;
        if (!check(event.timestamp_utc == "2026-09-14T17:00:00Z", "timestamp")) return 1;
        if (!check(event.source_id == "SRC-14", "source_id")) return 1;
        if (!check(event.parameter == "c2.link_state", "parameter")) return 1;
        if (!check(event.severity == EventSeverity::WARNING, "severity")) return 1;
        if (!check(event.state == EventState::ACTIVE, "state")) return 1;
        std::cout << "PASS: final\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "EXCEPTION: " << ex.what() << '\n';
        return 2;
    }
}
