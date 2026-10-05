#pragma once

#include <cstdint>
#include <string>

namespace bluesky::flight_record {

struct FlightRecordReference { std::string flightRecordId; };

class FlightRecordProducer final {
public:
    explicit FlightRecordProducer(std::string flightRecordId);
    FlightRecordReference begin() const;
    std::int64_t timestampMs() const;
private:
    std::string flightRecordId_;
};

} // namespace bluesky::flight_record
