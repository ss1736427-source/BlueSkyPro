#include "command_lifecycle_runtime.hpp"

#include <cassert>

using namespace bluesky::planning::command;

namespace {
CommandRequest makeRequest() {
    return {"CMD-019-001", "MISSION-019", "UAV-019", "TAKEOFF", "{}", "AUTHORITY_GATE", 1000, "CORR-019"};
}
}

int main() {
    const CommandRequest request = makeRequest();

    CommandLifecycleRuntime runtime(RetryPolicy{2, 100, true});
    assert(runtime.validate(request).state == CommandState::Validating);
    assert(runtime.dispatch(request).state == CommandState::Dispatched);
    assert(runtime.attemptCount() == 1);
    assert(!runtime.timeoutExpired(1099));
    assert(runtime.timeoutExpired(1100));

    assert(runtime.retry(request, 1100).state == CommandState::Dispatched);
    assert(runtime.attemptCount() == 2);
    assert(runtime.retryAllowed());
    assert(runtime.retry(request, 1200).state == CommandState::Dispatched);
    assert(runtime.attemptCount() == 3);
    assert(!runtime.retryAllowed());

    assert(runtime.acknowledge(request).state == CommandState::Acknowledged);
    assert(runtime.execute(request).state == CommandState::Executing);
    assert(runtime.complete(request).state == CommandState::Completed);
    assert(runtime.state() == CommandState::Completed);

    CommandLifecycleRuntime invalid;
    CommandRequest bad = request;
    bad.command_id.clear();
    assert(invalid.validate(bad).state == CommandState::Rejected);

    CommandLifecycleRuntime nonIdempotent(RetryPolicy{3, 100, false});
    assert(nonIdempotent.validate(request).state == CommandState::Validating);
    assert(nonIdempotent.dispatch(request).state == CommandState::Dispatched);
    assert(!nonIdempotent.retryAllowed());
    assert(nonIdempotent.retry(request, 1100).state == CommandState::Unknown);

    CommandLifecycleRuntime loss(RetryPolicy{1, 100, true});
    assert(loss.validate(request).state == CommandState::Validating);
    assert(loss.dispatch(request).state == CommandState::Dispatched);
    assert(loss.unknown(request, "LINK_LOST_BEFORE_ACK").state == CommandState::Unknown);

    return 0;
}
