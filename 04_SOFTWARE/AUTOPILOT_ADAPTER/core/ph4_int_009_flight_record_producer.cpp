#include "ph4_int_009_flight_record_producer.hpp"

#include <chrono>
#include <utility>

namespace bluesky::flight_record {

FlightRecordProducer::FlightRecordProducer(std::string flightRecordId)
    : flightRecordId_(std::move(flightRecordId)) {}

FlightRecordReference FlightRecordProducer::begin() const { return {flightRecordId_}; }

std::int64_t FlightRecordProducer::timestampMs() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

} // namespace bluesky::flight_record
