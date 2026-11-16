#pragma once

#include "model/mission_model.hpp"

#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace bluesky::planning {

// Owns the canonical in-process lifecycle records. All state changes are
// delegated to MissionLifecycleController; this class adds identity ownership,
// atomic persistence and duplicate-registration protection, not a second
// transition policy.
class MissionLifecycleOwner final {
public:
    bool registerMission(const Mission& mission, std::vector<std::string>* blockers = nullptr);

    // Idempotently registers or advances an editable DRAFT revision. Once
    // admission starts, the existing lifecycle record is immutable here.
    bool synchronizeDraftMission(const Mission& mission,
                                 std::vector<std::string>* blockers = nullptr);

    MissionLifecycleTransitionResult transition(
        const std::string& mission_id,
        MissionLifecycleState target,
        const MissionLifecycleEvidence& evidence,
        const std::string& actor_id,
        const std::string& timestamp,
        const std::string& reason);

    std::optional<MissionLifecycleRecord> snapshot(const std::string& mission_id) const;
    bool contains(const std::string& mission_id) const;

private:
    mutable std::mutex m_mutex;
    std::map<std::string, MissionLifecycleRecord> m_records;
};

} // namespace bluesky::planning
