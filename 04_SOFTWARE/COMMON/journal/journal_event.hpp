#pragma once
#include <string>

namespace bluesky::journal {

struct JournalEvent {
    std::string event_id;
    std::string event_type;
    std::string timestamp_utc;
    std::string mission_id;
    std::string mission_version;
    std::string uav_id;
    std::string triggering_condition;
    std::string assessment_factors;
    std::string system_recommendation;
    std::string operator_decision;
    std::string resulting_action;
    std::string operator_identity;
    std::string source_context;
};

} // namespace bluesky::journal
