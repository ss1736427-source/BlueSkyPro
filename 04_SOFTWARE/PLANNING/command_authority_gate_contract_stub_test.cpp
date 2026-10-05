#include "command_authority_gate_contract_stub.hpp"

#include <cassert>
#include <string>

using namespace bluesky::planning::authority;
using bluesky::planning::command::CommandRequest;

namespace {

class StubAuthorityGate final : public CommandAuthorityGateContract {
public:
    AuthorityDecisionResult evaluate(const AuthorityEvaluation& evaluation) override {
        AuthorityDecisionResult result;
        result.policy_version = evaluation.policy_version;
        result.evaluated_at_ms = evaluation.evaluated_at_ms;
        result.correlation_id = evaluation.correlation_id;
        if (evaluation.request.command_id == "ALLOW-001") {
            result.decision = AuthorityDecision::Allowed;
            result.reason = "controlled test allow";
        } else if (evaluation.request.command_id == "REJECT-001") {
            result.decision = AuthorityDecision::Rejected;
            result.reason = "controlled test reject";
        } else {
            result.decision = AuthorityDecision::Deferred;
            result.reason = "controlled test deferred";
        }
        return result;
    }
};

} // namespace

int main() {
    StubAuthorityGate gate;

    const auto allowed = gate.evaluate({CommandRequest{"ALLOW-001", "MISSION-1", "VEH-1", "ARM", "{}", "TEST", 100, "CORR-1"}, "POLICY-1", 110, "CORR-1"});
    assert(allowed.decision == AuthorityDecision::Allowed);
    assert(allowed.reason == "controlled test allow");
    assert(allowed.policy_version == "POLICY-1");
    assert(allowed.correlation_id == "CORR-1");

    const auto rejected = gate.evaluate({CommandRequest{"REJECT-001", "MISSION-1", "VEH-1", "ARM", "{}", "TEST", 100, "CORR-2"}, "POLICY-1", 120, "CORR-2"});
    assert(rejected.decision == AuthorityDecision::Rejected);
    assert(rejected.reason == "controlled test reject");
    assert(rejected.decision != AuthorityDecision::Allowed);

    const auto deferred = gate.evaluate({CommandRequest{"UNKNOWN-001", "MISSION-1", "VEH-1", "ARM", "{}", "TEST", 100, "CORR-3"}, "POLICY-1", 130, "CORR-3"});
    assert(deferred.decision == AuthorityDecision::Deferred);
    assert(deferred.reason == "controlled test deferred");
    assert(deferred.decision != AuthorityDecision::Allowed);

    return 0;
}
