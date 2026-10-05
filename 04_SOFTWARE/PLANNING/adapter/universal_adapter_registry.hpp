#pragma once

#include "universal_adapter.hpp"

#include <memory>
#include <string>
#include <vector>

namespace bluesky::planning::adapter {

enum class ResolutionStatus {
    Resolved,
    NotFound,
    Incompatible,
    CapabilityUnsupported,
    Ambiguous,
};

struct AdapterResolutionRequest {
    std::string vehicle_profile;
    std::string equipment_profile;
    std::string protocol;
    std::string contract_version;
    std::string schema_version;
    std::string required_capability;
};

struct AdapterResolutionResult {
    ResolutionStatus status{};
    UniversalAdapter* adapter{nullptr};
};

class UniversalAdapterRegistry {
public:
    bool registerAdapter(std::unique_ptr<UniversalAdapter> adapter);
    bool unregisterAdapter(const std::string& adapter_id);

    UniversalAdapter* findById(const std::string& adapter_id) const;
    UniversalAdapter* findForVehicleProfile(const std::string& vehicle_profile) const;
    UniversalAdapter* findForEquipmentProfile(const std::string& equipment_profile) const;
    UniversalAdapter* findForCapability(const std::string& capability) const;
    AdapterResolutionResult resolve(const AdapterResolutionRequest& request) const;

    std::vector<std::string> adapterIds() const;

private:
    std::vector<std::unique_ptr<UniversalAdapter>> adapters_;
};

} // namespace bluesky::planning::adapter
