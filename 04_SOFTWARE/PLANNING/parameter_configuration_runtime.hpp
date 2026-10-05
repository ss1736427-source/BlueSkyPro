#pragma once

#include "../AUTOPILOT_ADAPTER/core/universal_autopilot_adapter.hpp"

#include <optional>
#include <string>
#include <vector>

namespace bluesky::planning {

enum class ParameterSyncState {
    Rejected,
    Read,
    Compared,
    BackedUp,
    Written,
    Restored,
    Verified,
    Failed
};

struct ParameterConfigurationSnapshot {
    std::string vehicle_id;
    std::string autopilot;
    std::string firmware_version;
    std::string protocol;
    std::string protocol_version;
    std::string configuration;
};

struct ParameterConfigurationResult {
    ParameterSyncState state{ParameterSyncState::Rejected};
    ParameterConfigurationSnapshot snapshot{};
    std::optional<ParameterConfigurationSnapshot> backup{};
    std::vector<std::string> mismatches{};
    std::string reason;
};

class ParameterConfigurationRuntime final {
public:
    static ParameterConfigurationResult read(
        bluesky::autopilot::UniversalAutopilotAdapter& adapter);

    static ParameterConfigurationResult compare(
        const ParameterConfigurationSnapshot& approved,
        bluesky::autopilot::UniversalAutopilotAdapter& adapter);

    static ParameterConfigurationResult backup(
        bluesky::autopilot::UniversalAutopilotAdapter& adapter);

    static ParameterConfigurationResult write(
        const ParameterConfigurationSnapshot& approved,
        bluesky::autopilot::UniversalAutopilotAdapter& adapter);

    static ParameterConfigurationResult restore(
        const ParameterConfigurationSnapshot& backup,
        bluesky::autopilot::UniversalAutopilotAdapter& adapter);

    static ParameterConfigurationResult synchronize(
        const ParameterConfigurationSnapshot& approved,
        bluesky::autopilot::UniversalAutopilotAdapter& adapter);

    static ParameterConfigurationResult verify(
        const ParameterConfigurationSnapshot& approved,
        bluesky::autopilot::UniversalAutopilotAdapter& adapter);

private:
    static ParameterConfigurationSnapshot identitySnapshot(
        const bluesky::autopilot::Identity& identity,
        const std::string& configuration);

    static ParameterConfigurationResult reject(
        const std::string& reason);
};

} // namespace bluesky::planning
