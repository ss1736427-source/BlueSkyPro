#pragma once

#include <cstdint>
#include <string>

namespace bluesky::planning::command {

enum class CommandState {
    Requested,
    Validating,
    Rejected,
    Dispatched,
    Acknowledged,
    Executing,
    Completed,
    Failed,
    Cancelled,
    Timeout,
    Unknown
};

struct CommandRequest {
    std::string command_id;
    std::string mission_id;
    std::string target_vehicle_id;
    std::string command_class;
    std::string parameters;
    std::string authority_source;
    std::int64_t requested_at_ms{0};
    std::string correlation_id;
};

struct CommandTransition {
    CommandState state{CommandState::Requested};
    std::int64_t timestamp_ms{0};
    std::string reason;
};

struct CommandResult {
    CommandState state{CommandState::Unknown};
    bool vehicle_acknowledged{false};
    bool execution_confirmed{false};
    std::string reason;
};

class CommandLifecycleContract {
public:
    virtual ~CommandLifecycleContract() = default;

    virtual CommandTransition validate(const CommandRequest& request) = 0;
    virtual CommandTransition dispatch(const CommandRequest& request) = 0;
    virtual CommandTransition acknowledge(const CommandRequest& request) = 0;
    virtual CommandTransition execute(const CommandRequest& request) = 0;
    virtual CommandTransition complete(const CommandRequest& request) = 0;
    virtual CommandTransition fail(const CommandRequest& request, std::string reason) = 0;
    virtual CommandTransition cancel(const CommandRequest& request, std::string reason) = 0;
    virtual CommandTransition timeout(const CommandRequest& request) = 0;
    virtual CommandTransition unknown(const CommandRequest& request, std::string reason) = 0;
};

} // namespace bluesky::planning::command
