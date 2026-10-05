#include "command_lifecycle_contract_stub.hpp"

#include <cassert>
#include <type_traits>
#include <utility>

namespace {

class TestCommandLifecycle final : public bluesky::planning::command::CommandLifecycleContract {
public:
    bluesky::planning::command::CommandTransition validate(
        const bluesky::planning::command::CommandRequest&) override {
        return {bluesky::planning::command::CommandState::Validating, 10, ""};
    }

    bluesky::planning::command::CommandTransition dispatch(
        const bluesky::planning::command::CommandRequest&) override {
        return {bluesky::planning::command::CommandState::Dispatched, 20, ""};
    }

    bluesky::planning::command::CommandTransition acknowledge(
        const bluesky::planning::command::CommandRequest&) override {
        return {bluesky::planning::command::CommandState::Acknowledged, 30, ""};
    }

    bluesky::planning::command::CommandTransition execute(
        const bluesky::planning::command::CommandRequest&) override {
        return {bluesky::planning::command::CommandState::Executing, 35, ""};
    }

    bluesky::planning::command::CommandTransition complete(
        const bluesky::planning::command::CommandRequest&) override {
        return {bluesky::planning::command::CommandState::Completed, 40, ""};
    }

    bluesky::planning::command::CommandTransition fail(
        const bluesky::planning::command::CommandRequest&, std::string reason) override {
        return {bluesky::planning::command::CommandState::Failed, 50, std::move(reason)};
    }

    bluesky::planning::command::CommandTransition cancel(
        const bluesky::planning::command::CommandRequest&, std::string reason) override {
        return {bluesky::planning::command::CommandState::Cancelled, 60, std::move(reason)};
    }

    bluesky::planning::command::CommandTransition timeout(
        const bluesky::planning::command::CommandRequest&) override {
        return {bluesky::planning::command::CommandState::Timeout, 70, ""};
    }

    bluesky::planning::command::CommandTransition unknown(
        const bluesky::planning::command::CommandRequest&, std::string reason) override {
        return {bluesky::planning::command::CommandState::Unknown, 80, std::move(reason)};
    }
};

} // namespace

int main() {
    static_assert(std::is_abstract_v<bluesky::planning::command::CommandLifecycleContract>);

    TestCommandLifecycle lifecycle;
    const bluesky::planning::command::CommandRequest request{
        "CMD-TEST-001", "MISSION-001", "VEH-TEST-001", "TAKEOFF", "{}", "BLUE_SKY", 1, "CORR-001"};

    assert(request.command_id == "CMD-TEST-001");
    assert(request.mission_id == "MISSION-001");
    assert(request.target_vehicle_id == "VEH-TEST-001");
    assert(request.command_class == "TAKEOFF");
    assert(request.parameters == "{}");
    assert(request.authority_source == "BLUE_SKY");
    assert(request.requested_at_ms == 1);
    assert(request.correlation_id == "CORR-001");

    const auto validating = lifecycle.validate(request);
    const auto dispatched = lifecycle.dispatch(request);
    const auto acknowledged = lifecycle.acknowledge(request);
    const auto executing = lifecycle.execute(request);
    const auto completed = lifecycle.complete(request);

    assert(validating.state == bluesky::planning::command::CommandState::Validating);
    assert(dispatched.state == bluesky::planning::command::CommandState::Dispatched);
    assert(acknowledged.state == bluesky::planning::command::CommandState::Acknowledged);
    assert(executing.state == bluesky::planning::command::CommandState::Executing);
    assert(completed.state == bluesky::planning::command::CommandState::Completed);

    assert(lifecycle.fail(request, "test failure").state == bluesky::planning::command::CommandState::Failed);
    assert(lifecycle.fail(request, "test failure").reason == "test failure");
    assert(lifecycle.cancel(request, "test cancellation").state == bluesky::planning::command::CommandState::Cancelled);
    assert(lifecycle.cancel(request, "test cancellation").reason == "test cancellation");
    assert(lifecycle.timeout(request).state == bluesky::planning::command::CommandState::Timeout);
    assert(lifecycle.unknown(request, "link lost").state == bluesky::planning::command::CommandState::Unknown);
    assert(lifecycle.unknown(request, "link lost").reason == "link lost");

    const bluesky::planning::command::CommandResult acknowledged_result{
        bluesky::planning::command::CommandState::Acknowledged, true, false, "acknowledged"};
    assert(acknowledged_result.vehicle_acknowledged);
    assert(!acknowledged_result.execution_confirmed);

    const bluesky::planning::command::CommandResult result{
        bluesky::planning::command::CommandState::Completed, true, true, "confirmed"};
    assert(result.vehicle_acknowledged);
    assert(result.execution_confirmed);

    return 0;
}
