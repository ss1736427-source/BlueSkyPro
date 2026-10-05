#pragma once

#include "command_lifecycle_contract_stub.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace bluesky::planning::command {

struct RetryPolicy {
    std::uint32_t max_retries{0};
    std::int64_t timeout_ms{0};
    bool idempotent{false};
};

class CommandLifecycleRuntime final : public CommandLifecycleContract {
public:
    explicit CommandLifecycleRuntime(RetryPolicy policy = {});

    CommandTransition validate(const CommandRequest& request) override;
    CommandTransition dispatch(const CommandRequest& request) override;
    CommandTransition acknowledge(const CommandRequest& request) override;
    CommandTransition execute(const CommandRequest& request) override;
    CommandTransition complete(const CommandRequest& request) override;
    CommandTransition fail(const CommandRequest& request, std::string reason) override;
    CommandTransition cancel(const CommandRequest& request, std::string reason) override;
    CommandTransition timeout(const CommandRequest& request) override;
    CommandTransition unknown(const CommandRequest& request, std::string reason) override;

    CommandTransition retry(const CommandRequest& request, std::int64_t now_ms);
    bool timeoutExpired(std::int64_t now_ms) const;
    bool retryAllowed() const;
    CommandState state() const;
    std::uint32_t attemptCount() const;
    const std::optional<CommandRequest>& request() const;

private:
    CommandTransition transition(CommandState next, std::int64_t timestamp_ms, std::string reason);
    bool matches(const CommandRequest& request) const;
    bool terminal() const;

    RetryPolicy policy_;
    std::optional<CommandRequest> request_;
    CommandState state_{CommandState::Requested};
    std::int64_t last_transition_ms_{0};
    std::uint32_t attempts_{0};
};

} // namespace bluesky::planning::command
