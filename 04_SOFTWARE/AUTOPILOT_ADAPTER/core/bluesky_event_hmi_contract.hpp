#pragma once

#include <stdexcept>
#include <string>

namespace bluesky::hmi {

enum class EventSeverity { INFO, NOTICE, WARNING, CRITICAL };
enum class EventState { ACTIVE, CLEARED, ACKNOWLEDGED };

struct BlueSkyEvent {
    std::string event_id;
    std::string timestamp_utc;
    std::string source_id;
    std::string parameter;
    std::string value;
    EventSeverity severity{EventSeverity::INFO};
    EventState state{EventState::ACTIVE};
};

class BlueSkyEventHmiContract final {
public:
    static void validate(const BlueSkyEvent& event) {
        if (event.event_id.empty()) throw std::invalid_argument("event_id is required");
        if (event.timestamp_utc.empty()) throw std::invalid_argument("timestamp_utc is required");
        if (event.source_id.empty()) throw std::invalid_argument("source_id is required");
        if (event.parameter.empty()) throw std::invalid_argument("parameter is required");
    }
};

} // namespace bluesky::hmi
