#pragma once
#include "ai_orchestration_boundary.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace bluesky::ai {
enum class TaskState { Created, Assigned, Running, Completed, TimedOut, Failed, Unavailable, Conflicted };
enum class AgentHealth { Available, Busy, Unavailable, Failed };
struct AgentRegistration { AgentIdentity identity; std::unordered_set<std::string> capabilities; AgentHealth health{AgentHealth::Available}; };
struct AgentResult { std::string task_id; std::string agent_id; std::string correlation_id; std::string result_id; std::string result_digest; bool valid{true}; bool failed{false}; std::string failure_reason; };
struct OrchestrationTask {
 std::string task_id; std::string correlation_id; std::string required_capability;
 std::vector<std::string> candidate_agent_ids; std::unordered_set<std::string> assigned_agent_ids;
 std::vector<AgentResult> results; TaskState state{TaskState::Created}; std::uint64_t deadline_tick{0};
};
class AiOrchestrator final {
public:
 bool register_agent(const AgentRegistration&);
 bool set_agent_health(const std::string&, AgentHealth);
 bool create_task(const OrchestrationTask&);
 bool assign_task(const std::string&);
 bool record_result(const AgentResult&);
 bool advance_time(std::uint64_t);
 const OrchestrationTask* task(const std::string&) const noexcept;
 std::size_t agent_count() const noexcept { return agents_.size(); }
 std::size_t task_count() const noexcept { return tasks_.size(); }
private:
 void recompute_task_state(OrchestrationTask&);
 std::unordered_map<std::string, AgentRegistration> agents_;
 std::unordered_map<std::string, OrchestrationTask> tasks_;
 std::uint64_t current_tick_{0};
};
} // namespace bluesky::ai
