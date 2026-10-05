#include "ph4_int_009_flight_record_producer.hpp"
#include "ph4_int_011_flight_record_lifecycle.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

namespace {
const char* stateName(bluesky::flight_record::LifecycleState state) {
    using bluesky::flight_record::LifecycleState;
    switch (state) {
    case LifecycleState::Init: return "INIT";
    case LifecycleState::Preparing: return "PREPARING";
    case LifecycleState::Active: return "ACTIVE";
    case LifecycleState::Completed: return "COMPLETED";
    case LifecycleState::Aborted: return "ABORTED";
    }
    return "UNKNOWN";
}
}

int main(int argc, char** argv) {
    const std::string output = argc > 1 ? argv[1] : "ph4_int_011_results.jsonl";
    std::ofstream out(output);
    assert(out.good());

    using namespace bluesky::flight_record;
    FlightRecordProducer producer("FLIGHT-RECORD-PH4-INT-011");
    FlightRecordLifecycle lifecycle(producer.begin());

    const auto preparation = lifecycle.beginPreparation();
    const auto started = lifecycle.beginExecution();
    const auto completed = lifecycle.completeExecution();

    assert(lifecycle.state() == LifecycleState::Completed);
    assert(lifecycle.events().size() == 3);
    assert(preparation.flightRecordId == started.flightRecordId);
    assert(started.flightRecordId == completed.flightRecordId);
    assert(preparation.timestampMs > 0);
    assert(preparation.timestampMs <= started.timestampMs);
    assert(started.timestampMs <= completed.timestampMs);

    for (const auto& event : lifecycle.events()) {
        out << "{\"timestamp_ms\":" << event.timestampMs
            << ",\"parameter\":\"flight_record_lifecycle\",\"value\":\"" << event.eventType
            << "\",\"unit\":\"none\",\"source\":\"BLUESKY_EVENT\",\"quality\":\"VALID\",\"context\":{\"flight_record_id\":\""
            << event.flightRecordId << "\",\"evidence_domain_id\":\"EC-01\",\"lifecycle_state\":\""
            << stateName(event.state) << "\",\"event_type\":\"" << event.eventType << "\"}}\n";
    }

    std::cout << "PH4_INT_011_FLIGHT_RECORD_LIFECYCLE: PASS\n";
    std::cout << "controlled_state_sequence=INIT->PREPARING->ACTIVE->COMPLETED\n";
    std::cout << "mission_execution_events=3\n";
    std::cout << "flight_record_binding=PASS\n";
    std::cout << "verification_status=FIXTURE_ONLY\n";
    return 0;
}
