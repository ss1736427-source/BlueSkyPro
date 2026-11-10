#include "weather_provider_registry.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace bluesky::weather {
namespace {

void setError(std::string *target, const char *message)
{
    if (target)
        *target = message;
}

bool validBox(const GeographicCoverage &box)
{
    return std::isfinite(box.west) && std::isfinite(box.south)
        && std::isfinite(box.east) && std::isfinite(box.north)
        && box.west >= -180.0 && box.east <= 180.0 && box.west < box.east
        && box.south >= -90.0 && box.north <= 90.0 && box.south < box.north;
}

bool contains(const GeographicCoverage &outer, const GeographicCoverage &inner)
{
    return outer.west <= inner.west && outer.south <= inner.south
        && outer.east >= inner.east && outer.north >= inner.north;
}

bool covers(const WeatherProviderCapability &capability,
            const WeatherProviderRequest &request)
{
    if (capability.product != request.product
        || !contains(capability.coverage, request.area))
        return false;
    if (!std::isfinite(capability.minimumAltitudeAglM)
        || !std::isfinite(capability.maximumAltitudeAglM)
        || !std::isfinite(request.minimumAltitudeAglM)
        || !std::isfinite(request.maximumAltitudeAglM)
        || capability.minimumAltitudeAglM > request.minimumAltitudeAglM
        || capability.maximumAltitudeAglM < request.maximumAltitudeAglM
        || request.minimumAltitudeAglM > request.maximumAltitudeAglM)
        return false;
    if (request.referenceTimeUtcSeconds <= 0
        || request.validFromUtcSeconds < request.referenceTimeUtcSeconds
        || request.validToUtcSeconds < request.validFromUtcSeconds
        || capability.maximumForecastHorizonSeconds < 0)
        return false;
    if (request.validToUtcSeconds - request.referenceTimeUtcSeconds
        > capability.maximumForecastHorizonSeconds)
        return false;
    if (!std::isfinite(request.maximumGridSpacingDegrees)
        || request.maximumGridSpacingDegrees < 0.0)
        return false;
    if (request.maximumGridSpacingDegrees > 0.0
        && (!std::isfinite(capability.minimumGridSpacingDegrees)
            || capability.minimumGridSpacingDegrees <= 0.0
            || capability.minimumGridSpacingDegrees > request.maximumGridSpacingDegrees))
        return false;
    return true;
}

} // namespace

bool WeatherProviderRegistry::registerAdapter(
    std::shared_ptr<const IWeatherProviderAdapter> adapter, std::string *error)
{
    if (!adapter) {
        setError(error, "adapter is null");
        return false;
    }
    const auto descriptor = adapter->descriptor();
    if (descriptor.adapterId.empty() || descriptor.displayName.empty()
        || descriptor.interfaceVersion != 1 || descriptor.capabilities.empty()) {
        setError(error, "adapter descriptor is incomplete or has an unsupported interface version");
        return false;
    }
    for (const auto &capability : descriptor.capabilities) {
        if (!validBox(capability.coverage)
            || !std::isfinite(capability.minimumAltitudeAglM)
            || !std::isfinite(capability.maximumAltitudeAglM)
            || capability.minimumAltitudeAglM > capability.maximumAltitudeAglM
            || capability.maximumForecastHorizonSeconds < 0) {
            setError(error, "adapter contains an invalid capability declaration");
            return false;
        }
    }
    const auto duplicate = std::find_if(m_adapters.begin(), m_adapters.end(),
        [&descriptor](const auto &existing) {
            return existing->descriptor().adapterId == descriptor.adapterId;
        });
    if (duplicate != m_adapters.end()) {
        setError(error, "adapter id is already registered");
        return false;
    }
    m_adapters.push_back(std::move(adapter));
    setError(error, "");
    return true;
}

bool WeatherProviderRegistry::upsertInstance(WeatherProviderInstance instance,
                                              std::string *error)
{
    if (instance.instanceId.empty() || instance.adapterId.empty()
        || instance.displayName.empty()) {
        setError(error, "provider instance identity is incomplete");
        return false;
    }
    const auto adapter = std::find_if(m_adapters.begin(), m_adapters.end(),
        [&instance](const auto &candidate) {
            return candidate->descriptor().adapterId == instance.adapterId;
        });
    if (adapter == m_adapters.end()) {
        setError(error, "provider instance references an unregistered adapter");
        return false;
    }
    const auto existing = std::find_if(m_instances.begin(), m_instances.end(),
        [&instance](const auto &candidate) {
            return candidate.instanceId == instance.instanceId;
        });
    if (existing == m_instances.end())
        m_instances.push_back(std::move(instance));
    else
        *existing = std::move(instance);
    setError(error, "");
    return true;
}

bool WeatherProviderRegistry::setEnabled(const std::string &instanceId, bool enabled)
{
    const auto it = std::find_if(m_instances.begin(), m_instances.end(),
        [&instanceId](const auto &instance) { return instance.instanceId == instanceId; });
    if (it == m_instances.end())
        return false;
    it->enabled = enabled;
    return true;
}

std::vector<WeatherProviderSelection> WeatherProviderRegistry::selectEligible(
    const WeatherProviderRequest &request) const
{
    std::vector<WeatherProviderSelection> result;
    if (!validBox(request.area))
        return result;

    for (const auto &instance : m_instances) {
        if (!instance.enabled)
            continue;
        const auto adapter = std::find_if(m_adapters.begin(), m_adapters.end(),
            [&instance](const auto &candidate) {
                return candidate->descriptor().adapterId == instance.adapterId;
            });
        if (adapter == m_adapters.end())
            continue;
        const auto descriptor = (*adapter)->descriptor();
        for (std::size_t i = 0; i < descriptor.capabilities.size(); ++i) {
            if (covers(descriptor.capabilities[i], request)) {
                result.push_back({instance, descriptor, i});
                break;
            }
        }
    }
    std::stable_sort(result.begin(), result.end(),
        [](const auto &left, const auto &right) {
            if (left.instance.priority != right.instance.priority)
                return left.instance.priority < right.instance.priority;
            return left.instance.instanceId < right.instance.instanceId;
        });
    return result;
}

std::vector<WeatherProviderInstance> WeatherProviderRegistry::instances() const
{
    return m_instances;
}

} // namespace bluesky::weather
