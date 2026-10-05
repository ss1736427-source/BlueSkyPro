#pragma once

#include "universal_adapter_registry.hpp"

namespace bluesky::planning::adapter {

// Service-facing boundary for adapter resolution.
// Services depend on this façade and do not need to know registry storage details.
class AdapterServiceResolver {
public:
    explicit AdapterServiceResolver(const UniversalAdapterRegistry& registry)
        : registry_(registry) {}

    AdapterResolutionResult resolve(const AdapterResolutionRequest& request) const {
        return registry_.resolve(request);
    }

private:
    const UniversalAdapterRegistry& registry_;
};

} // namespace bluesky::planning::adapter
