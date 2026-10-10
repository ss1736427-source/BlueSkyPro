#include "ph4_int_009_flight_record_producer.hpp"

#include <cassert>
#include <iostream>

int main() {
    bluesky::flight_record::FlightRecordProducer producer("FLIGHT-RECORD-PH4-INT-009");
    const auto reference = producer.begin();
    assert(reference.flightRecordId == "FLIGHT-RECORD-PH4-INT-009");
    assert(producer.timestampMs() > 0);
    std::cout << "PH4_INT_009_FLIGHT_RECORD_PRODUCER: PASS\n";
    std::cout << "source_reference=PASS\n";
    std::cout << "source_timestamp=PASS\n";
    std::cout << "verification_status=FIXTURE_ONLY\n";
    return 0;
}
