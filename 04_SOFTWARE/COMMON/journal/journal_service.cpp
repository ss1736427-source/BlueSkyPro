#include "journal_service.hpp"

namespace bluesky::journal {

bool JournalService::recordUavDecision(const JournalEvent& event) {
    if (event.event_type != "UAV_DECISION") {
        return false;
    }
    if (event.mission_id.empty() || event.uav_id.empty() ||
        event.operator_decision.empty()) {
        return false;
    }
    return store_.append(event);
}

} // namespace bluesky::journal
