#include "command_lifecycle_runtime.hpp"

#include <utility>

namespace bluesky::planning::command {

CommandLifecycleRuntime::CommandLifecycleRuntime(RetryPolicy policy) : policy_(policy) {}

CommandTransition CommandLifecycleRuntime::transition(
    CommandState next, std::int64_t timestamp_ms, std::string reason) {
    state_ = next;
    last_transition_ms_ = timestamp_ms;
    return {state_, timestamp_ms, std::move(reason)};
}

bool CommandLifecycleRuntime::matches(const CommandRequest& request) const {
    return request_.has_value()
        && request.command_id == request_->command_id
        && request.target_vehicle_id == request_->target_vehicle_id
        && request.correlation_id == request_->correlation_id;
}

bool CommandLifecycleRuntime::terminal() const {
    return state_ == CommandState::Rejected
        || state_ == CommandState::Completed
        || state_ == CommandState::Failed
        || state_ == CommandState::Cancelled
        || state_ == CommandState::Timeout;
}

CommandTransition CommandLifecycleRuntime::validate(const CommandRequest& request) {
    if (request.command_id.empty() || request.target_vehicle_id.empty()
        || request.command_class.empty() || request.authority_source.empty()
        || request.correlation_id.empty()) {
        return transition(CommandState::Rejected, request.requested_at_ms, "INVALID_COMMAND_IDENTITY");
    }
    if (terminal() && request_.has_value()) {
        return {state_, request.requested_at_ms, "COMMAND_LIFECYCLE_TERMINAL"};
    }
    request_ = request;
    return transition(CommandState::Validating, request.requested_at_ms, "VALIDATED_FOR_LIFECYCLE");
}

CommandTransition CommandLifecycleRuntime::dispatch(const CommandRequest& request) {
    if (!matches(request) || state_ != CommandState::Validating) {
        return transition(CommandState::Rejected, last_transition_ms_, "DISPATCH_NOT_ALLOWED");
    }
    ++attempts_;
    return transition(CommandState::Dispatched, request.requested_at_ms, "DISPATCHED");
}

CommandTransition CommandLifecycleRuntime::acknowledge(const CommandRequest& request) {
    if (!matches(request) || state_ != CommandState::Dispatched) {
        return transition(CommandState::Unknown, last_transition_ms_, "ACK_NOT_EXPECTED");
    }
    return transition(CommandState::Acknowledged, last_transition_ms_, "VEHICLE_ACKNOWLEDGED");
}

CommandTransition CommandLifecycleRuntime::execute(const CommandRequest& request) {
    if (!matches(request) || state_ != CommandState::Acknowledged) {
        return transition(CommandState::Unknown, last_transition_ms_, "EXECUTION_NOT_ALLOWED");
    }
    return transition(CommandState::Executing, last_transition_ms_, "EXECUTION_STARTED");
}

CommandTransition CommandLifecycleRuntime::complete(const CommandRequest& request) {
    if (!matches(request) || state_ != CommandState::Executing) {
        return transition(CommandState::Unknown, last_transition_ms_, "COMPLETION_NOT_ALLOWED");
    }
    return transition(CommandState::Completed, last_transition_ms_, "EXECUTION_CONFIRMED");
}

CommandTransition CommandLifecycleRuntime::fail(const CommandRequest& request, std::string reason) {
    if (!matches(request) || terminal()) {
        return transition(CommandState::Unknown, last_transition_ms_, "FAILURE_NOT_ALLOWED");
    }
    return transition(CommandState::Failed, last_transition_ms_, std::move(reason));
}

CommandTransition CommandLifecycleRuntime::cancel(const CommandRequest& request, std::string reason) {
    if (!matches(request) || terminal()) {
        return transition(CommandState::Unknown, last_transition_ms_, "CANCELLATION_NOT_ALLOWED");
    }
    return transition(CommandState::Cancelled, last_transition_ms_, std::move(reason));
}

CommandTransition CommandLifecycleRuntime::timeout(const CommandRequest& request) {
    if (!matches(request) || terminal()) {
        return transition(CommandState::Unknown, last_transition_ms_, "TIMEOUT_NOT_ALLOWED");
    }
    return transition(CommandState::Timeout, last_transition_ms_, "COMMAND_TIMEOUT");
}

CommandTransition CommandLifecycleRuntime::unknown(const CommandRequest& request, std::string reason) {
    if (!matches(request) || terminal()) {
        return transition(CommandState::Unknown, last_transition_ms_, "UNKNOWN_NOT_ALLOWED");
    }
    return transition(CommandState::Unknown, last_transition_ms_, std::move(reason));
}

CommandTransition CommandLifecycleRuntime::retry(
    const CommandRequest& request, std::int64_t now_ms) {
    if (!matches(request) || state_ != CommandState::Dispatched || !retryAllowed()) {
        return transition(CommandState::Unknown, now_ms, "RETRY_NOT_ALLOWED");
    }
    ++attempts_;
    return transition(CommandState::Dispatched, now_ms, "RETRY_DISPATCHED");
}

bool CommandLifecycleRuntime::timeoutExpired(std::int64_t now_ms) const {
    return state_ == CommandState::Dispatched
        && policy_.timeout_ms > 0
        && now_ms >= last_transition_ms_ + policy_.timeout_ms;
}

bool CommandLifecycleRuntime::retryAllowed() const {
    return state_ == CommandState::Dispatched
        && policy_.idempotent
        && attempts_ <= policy_.max_retries;
}

CommandState CommandLifecycleRuntime::state() const { return state_; }

std::uint32_t CommandLifecycleRuntime::attemptCount() const { return attempts_; }

const std::optional<CommandRequest>& CommandLifecycleRuntime::request() const { return request_; }

} // namespace bluesky::planning::command
