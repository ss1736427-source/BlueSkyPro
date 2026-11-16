#include "mission_lifecycle_owner.hpp"
#include "mission_lifecycle_validation_coordinator.hpp"

#include <charconv>
#include <utility>

namespace bluesky::planning {
namespace {
void addBlocker(std::vector<std::string>* blockers, const char* code)
{
    if (blockers)
        blockers->emplace_back(code);
}

bool numericVersion(const std::string& value, std::uint64_t& parsed)
{
    if (value.empty())
        return false;
    const auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
    return result.ec == std::errc{} && result.ptr == value.data() + value.size();
}

bool versionAdvances(const std::string& current, const std::string& next)
{
    std::uint64_t currentNumber = 0;
    std::uint64_t nextNumber = 0;
    if (numericVersion(current, currentNumber) && numericVersion(next, nextNumber))
        return nextNumber > currentNumber;
    return current != next;
}
}

bool MissionLifecycleOwner::registerMission(
    const Mission& mission, std::vector<std::string>* blockers)
{
    if (blockers)
        blockers->clear();
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

bool MissionLifecycleOwner::synchronizeDraftMission(
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
    const auto it = m_records.find(lifecycle.mission_id);
    if (it == m_records.end()) {
        m_records.emplace(lifecycle.mission_id, lifecycle);
        if (blockers)
            blockers->clear();
        return true;
    }

    auto& current = it->second;
    if (current.state != MissionLifecycleState::Draft || !current.history.empty()
        || !current.vehicle_id.empty() || current.next_event_sequence != 1) {
        addBlocker(blockers, "MISSION_DRAFT_REVISION_NOT_ALLOWED_AFTER_ADMISSION");
        return false;
    }

    if (current.mission_version == lifecycle.mission_version) {
        if (current.mission_template_ids != lifecycle.mission_template_ids) {
            addBlocker(blockers, "MISSION_DRAFT_VERSION_NOT_ADVANCED");
            return false;
        }
        if (blockers)
            blockers->clear();
        return true;
    }
    if (!versionAdvances(current.mission_version, lifecycle.mission_version)) {
        addBlocker(blockers, "MISSION_DRAFT_VERSION_NOT_ADVANCED");
        return false;
    }

    current.parent_mission_version = current.mission_version;
    current.mission_version = lifecycle.mission_version;
    current.mission_template_ids = lifecycle.mission_template_ids;
    if (blockers)
        blockers->clear();
    return true;
}


MissionLifecycleTransitionResult MissionLifecycleOwner::finalizeReadyFromAuthoritativeSources(
    const std::string& mission_id,
    const MissionLifecycleEvidence& evidence,
    const MissionLifecycleAuthoritativeSources& sources,
    const RegulatoryComplianceAssessment& regulatory_assessment,
    std::uint64_t now_epoch_s,
    const FlightProfile& selected_flight_profile,
    const VehicleCompilationInput& assigned_vehicle,
    const std::string& actor_id,
    const std::string& timestamp,
    const std::string& reason,
    const MissionLifecycleMt01QualityInputs* mt01_quality)
{
    MissionLifecycleRecord candidate;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_records.find(mission_id);
        if (it == m_records.end()) {
            MissionLifecycleTransitionResult missing;
            missing.blockers.emplace_back("MISSION_NOT_REGISTERED");
            return missing;
        }
        candidate = it->second;
    }

    const MissionLifecycleRecord baseline = candidate;
    std::optional<MissionLifecycleMt01QualityInputs> quality;
    if (mt01_quality)
        quality = *mt01_quality;
    const auto coordinated =
        MissionLifecycleValidationCoordinator::finalizeReadyFromAuthoritativeSources(
            candidate, evidence, sources, regulatory_assessment, now_epoch_s,
            selected_flight_profile, assigned_vehicle, actor_id, timestamp,
            reason, quality);
    auto result = coordinated.finalization.transition;
    if (!result.accepted)
        return result;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_records.find(mission_id);
        if (it == m_records.end()) {
            result.accepted = false;
            result.blockers.emplace_back("MISSION_NOT_REGISTERED");
            return result;
        }
        const auto& current = it->second;
        const bool unchanged = current.mission_version == baseline.mission_version
            && current.state == baseline.state
            && current.next_event_sequence == baseline.next_event_sequence
            && current.current_route_id == baseline.current_route_id
            && current.current_route_version == baseline.current_route_version
            && current.current_calculation_input_version == baseline.current_calculation_input_version
            && current.history.size() == baseline.history.size();
        if (!unchanged) {
            result.accepted = false;
            result.state = current.state;
            result.blockers.emplace_back("LIFECYCLE_RECORD_CHANGED_DURING_FINALIZATION");
            return result;
        }
        it->second = std::move(candidate);
    }
    return result;
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
