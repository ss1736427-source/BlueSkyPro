#include "authority_automation_resolution.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    const AuthorityContext full{true, true, true};

    Action automatic{"auto", {}, AutomationLevel::Automatic, true, true, true, true, 100};
    auto result = resolve_execution(automatic, full);
    assert(result.status == ResolutionStatus::Executable);
    assert(result.authority == Authority::System);

    const AuthorityContext no_system{false, true, true};
    result = resolve_execution(automatic, no_system);
    assert(result.status == ResolutionStatus::RequiresHuman);
    assert(result.authority == Authority::Role);
    assert(result.automation == AutomationLevel::HumanGuided);

    Action guided{"guided", {}, AutomationLevel::HumanGuided, true, true, true, true, 90};
    result = resolve_execution(guided, no_system);
    assert(result.status == ResolutionStatus::RequiresHuman);
    assert(result.authority == Authority::Role);

    const AuthorityContext no_role{false, false, true};
    result = resolve_execution(guided, no_role);
    assert(result.status == ResolutionStatus::Blocked);

    Action decision{"decision", {}, AutomationLevel::HumanDecision, true, true, true, true, 80};
    result = resolve_execution(decision, full);
    assert(result.status == ResolutionStatus::Executable);
    assert(result.authority == Authority::ExplicitHumanDecision);

    const AuthorityContext pending_decision{true, true, false};
    result = resolve_execution(decision, pending_decision);
    assert(result.status == ResolutionStatus::RequiresHuman);

    Action unsafe{"unsafe", {}, AutomationLevel::Automatic, true, false, true, true, 100};
    result = resolve_execution(unsafe, full);
    assert(result.status == ResolutionStatus::Blocked);
    assert(result.reason == "safety_gate");

    Action unauthorized{"unauthorized", {}, AutomationLevel::Automatic, false, true, true, true, 100};
    result = resolve_execution(unauthorized, full);
    assert(result.status == ResolutionStatus::Blocked);
    assert(result.reason == "action_not_authorized");

    return 0;
}
