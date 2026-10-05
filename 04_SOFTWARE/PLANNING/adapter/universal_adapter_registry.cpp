#include "universal_adapter_registry.hpp"

#include <algorithm>

namespace bluesky::planning::adapter {

namespace {

bool contains(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

bool matchesTarget(const AdapterMetadata& metadata, const AdapterResolutionRequest& request) {
    if (!request.vehicle_profile.empty() &&
        !contains(metadata.supported_vehicle_profiles, request.vehicle_profile)) {
        return false;
    }

    if (!request.equipment_profile.empty() &&
        !contains(metadata.supported_equipment_profiles, request.equipment_profile)) {
        return false;
    }

    return true;
}

bool matchesCompatibility(const AdapterMetadata& metadata, const AdapterResolutionRequest& request) {
    if (!request.protocol.empty() && metadata.protocol != request.protocol) return false;
    if (!request.contract_version.empty() && metadata.contract_version != request.contract_version) return false;
    if (!request.schema_version.empty() && metadata.schema_version != request.schema_version) return false;
    return true;
}

bool matchesCapability(const AdapterMetadata& metadata, const AdapterResolutionRequest& request) {
    return request.required_capability.empty() || contains(metadata.capabilities, request.required_capability);
}

} // namespace

bool UniversalAdapterRegistry::registerAdapter(std::unique_ptr<UniversalAdapter> adapter) {
    if (!adapter) return false;

    const auto id = adapter->metadata().adapter_id;
    if (id.empty() || findById(id) != nullptr) return false;

    adapters_.push_back(std::move(adapter));
    return true;
}

bool UniversalAdapterRegistry::unregisterAdapter(const std::string& adapter_id) {
    if (adapter_id.empty()) return false;

    const auto it = std::find_if(
        adapters_.begin(), adapters_.end(),
        [&](const std::unique_ptr<UniversalAdapter>& adapter) {
            return adapter && adapter->metadata().adapter_id == adapter_id;
        });

    if (it == adapters_.end()) return false;
    adapters_.erase(it);
    return true;
}

UniversalAdapter* UniversalAdapterRegistry::findById(const std::string& adapter_id) const {
    if (adapter_id.empty()) return nullptr;

    const auto it = std::find_if(
        adapters_.begin(), adapters_.end(),
        [&](const std::unique_ptr<UniversalAdapter>& adapter) {
            return adapter && adapter->metadata().adapter_id == adapter_id;
        });

    return it == adapters_.end() ? nullptr : it->get();
}

UniversalAdapter* UniversalAdapterRegistry::findForVehicleProfile(
    const std::string& vehicle_profile) const {
    if (vehicle_profile.empty()) return nullptr;

    UniversalAdapter* match = nullptr;
    std::size_t match_count = 0;
    for (const auto& adapter : adapters_) {
        if (!adapter) continue;
        const auto& profiles = adapter->metadata().supported_vehicle_profiles;
        if (std::find(profiles.begin(), profiles.end(), vehicle_profile) == profiles.end()) continue;
        match = adapter.get();
        ++match_count;
    }

    return match_count == 1 ? match : nullptr;
}

UniversalAdapter* UniversalAdapterRegistry::findForEquipmentProfile(
    const std::string& equipment_profile) const {
    if (equipment_profile.empty()) return nullptr;

    UniversalAdapter* match = nullptr;
    std::size_t match_count = 0;
    for (const auto& adapter : adapters_) {
        if (!adapter) continue;
        const auto& profiles = adapter->metadata().supported_equipment_profiles;
        if (std::find(profiles.begin(), profiles.end(), equipment_profile) == profiles.end()) continue;
        match = adapter.get();
        ++match_count;
    }

    return match_count == 1 ? match : nullptr;
}

UniversalAdapter* UniversalAdapterRegistry::findForCapability(
    const std::string& capability) const {
    if (capability.empty()) return nullptr;

    UniversalAdapter* match = nullptr;
    std::size_t match_count = 0;
    for (const auto& adapter : adapters_) {
        if (!adapter) continue;
        const auto& capabilities = adapter->metadata().capabilities;
        if (std::find(capabilities.begin(), capabilities.end(), capability) == capabilities.end()) continue;
        match = adapter.get();
        ++match_count;
    }

    return match_count == 1 ? match : nullptr;
}

AdapterResolutionResult UniversalAdapterRegistry::resolve(
    const AdapterResolutionRequest& request) const {
    const UniversalAdapter* first_compatible = nullptr;
    std::size_t compatible_count = 0;
    bool target_found = false;
    bool capability_mismatch = false;

    for (const auto& adapter : adapters_) {
        if (!adapter) continue;

        const auto metadata = adapter->metadata();
        if (!matchesTarget(metadata, request)) continue;

        target_found = true;
        if (!matchesCompatibility(metadata, request)) continue;

        if (!matchesCapability(metadata, request)) {
            capability_mismatch = true;
            continue;
        }

        first_compatible = adapter.get();
        ++compatible_count;
    }

    if (compatible_count == 1) {
        return {ResolutionStatus::Resolved, const_cast<UniversalAdapter*>(first_compatible)};
    }

    if (compatible_count > 1) {
        return {ResolutionStatus::Ambiguous, nullptr};
    }

    if (!target_found) {
        return {ResolutionStatus::NotFound, nullptr};
    }

    if (capability_mismatch) {
        return {ResolutionStatus::CapabilityUnsupported, nullptr};
    }

    return {ResolutionStatus::Incompatible, nullptr};
}

std::vector<std::string> UniversalAdapterRegistry::adapterIds() const {
    std::vector<std::string> result;
    result.reserve(adapters_.size());

    for (const auto& adapter : adapters_) {
        if (adapter) result.push_back(adapter->metadata().adapter_id);
    }

    return result;
}

} // namespace bluesky::planning::adapter
