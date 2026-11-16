#include "mission_lifecycle_owner.hpp"

#include <utility>

namespace bluesky::planning {
namespace {
void addBlocker(std::vector<std::string>* blockers, const char* code)
{
    if (blockers)
        blockers->emplace_back(code);
}
}

bool MissionLifecycleOwner::registerMission(
    const Mission& mission, std::vector<std::string>* blockers)
{
    std::vector<std::string> localBlockers;
    if (mission.identity.mission_id.empty())
        localBlockers.emplace_back("MISSION_ID_REQUIRED");
    if (mission.identity.mission_version.empty())
        localBlockers.emplace_back("MISSION_VERSION_REQUIRED");

    Mission canonical = mission;
    synchronizeMissionLifecycleIdentity(canonical);
    const auto& supplied = mission.lifecycle;
    const auto& lifecycle = canonical.lifecycle;

    if ((!supplied.mission_id.empty() && supplied.mission_id != lifecycle.mission_id)
        || (!supplied.mission_version.empty()
            && supplied.mission_version != lifecycle.mission_version))
        localBlockers.emplace_back("MISSION_LIFECYCLE_IDENTITY_MISMATCH");
    if (supplied.state != MissionLifecycleState::Draft || !supplied.history.empty()
        || !supplied.vehicle_id.empty() || supplied.next_event_sequence != 1)
        localBlockers.emplace_back("MISSION_LIFECYCLE_MUST_START_AS_UNBOUND_DRAFT");

    if (!localBlockers.empty()) {
        if (blockers)
            *blockers = std::move(localBlockers);
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_records.find(lifecycle.mission_id) != m_records.end()) {
        addBlocker(blockers, "MISSION_ALREADY_REGISTERED");
        return false;
    }

    m_records.emplace(lifecycle.mission_id, lifecycle);
    if (blockers)
        blockers->clear();
    return true;
}

MissionLifecycleTransitionResult MissionLifecycleOwner::transition(
    const std::string& mission_id,
    MissionLifecycleState target,
    const MissionLifecycleEvidence& evidence,
    const std::string& actor_id,
    const std::string& timestamp,
    const std::string& reason)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_records.find(mission_id);
    if (it == m_records.end()) {
        MissionLifecycleTransitionResult result;
        result.blockers.emplace_back("MISSION_NOT_REGISTERED");
        return result;
    }

    // Work on a copy. Rejected transitions cannot partially mutate the
    // authoritative record, even if validation discovers several blockers.
    MissionLifecycleRecord candidate = it->second;
    auto result = MissionLifecycleController::transition(
        candidate, target, evidence, actor_id, timestamp, reason);
    if (result.accepted)
        it->second = std::move(candidate);
    return result;
}

std::optional<MissionLifecycleRecord> MissionLifecycleOwner::snapshot(
    const std::string& mission_id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_records.find(mission_id);
    if (it == m_records.end())
        return std::nullopt;
    return it->second;
}

bool MissionLifecycleOwner::contains(const std::string& mission_id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_records.find(mission_id) != m_records.end();
}

} // namespace bluesky::planning
