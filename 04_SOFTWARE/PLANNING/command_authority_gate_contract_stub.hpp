#pragma once

#include "command_lifecycle_contract_stub.hpp"

#include <cstdint>
#include <string>

namespace bluesky::planning::authority {

enum class AuthorityDecision {
    Allowed,
    Rejected,
    Deferred
};

struct AuthorityEvaluation {
    bluesky::planning::command::CommandRequest request;
    std::string policy_version;
    std::int64_t evaluated_at_ms{0};
    std::string correlation_id;
};

struct AuthorityDecisionResult {
    AuthorityDecision decision{AuthorityDecision::Deferred};
    std::string reason;
    std::string policy_version;
    std::int64_t evaluated_at_ms{0};
    std::string correlation_id;
};

class CommandAuthorityGateContract {
public:
    virtual ~CommandAuthorityGateContract() = default;

    virtual AuthorityDecisionResult evaluate(const AuthorityEvaluation& evaluation) = 0;
};

} // namespace bluesky::planning::authority
