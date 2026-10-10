#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace bluesky::c2 {

enum class ChannelState {
    Available,
    Degraded,
    Unavailable,
    Recovering,
    Active,
    Standby,
    Failed
};

struct ChannelSnapshot {
    std::string id;
    std::string transport;
    ChannelState state{ChannelState::Unavailable};
    double latencyMs{0.0};
    double packetLoss{0.0};
    double capacityKbps{0.0};
    bool integrityOk{false};
    bool authenticated{false};
    int priority{0};
    std::int64_t lastSuccessTimestampMs{0};
};

enum class ChannelSelectionStatus {
    Selected,
    NoQualifiedChannel
};

struct ChannelSelection {
    ChannelSelectionStatus status{ChannelSelectionStatus::NoQualifiedChannel};
    std::string channelId;
    std::string reason;
};

class C2ChannelManager final {
public:
    void registerChannel(ChannelSnapshot channel) {
        const auto it = std::find_if(channels_.begin(), channels_.end(),
            [&](const ChannelSnapshot& current) { return current.id == channel.id; });
        if (it == channels_.end()) {
            channels_.push_back(std::move(channel));
        } else {
            *it = std::move(channel);
        }
    }

    bool update(ChannelSnapshot channel) {
        const auto it = std::find_if(channels_.begin(), channels_.end(),
            [&](const ChannelSnapshot& current) { return current.id == channel.id; });
        if (it == channels_.end()) return false;
        *it = std::move(channel);
        return true;
    }

    ChannelSelection selectBest(bool commandCritical) const {
        const ChannelSnapshot* best = nullptr;
        for (const auto& channel : channels_) {
            if (!qualified(channel, commandCritical)) continue;
            if (best == nullptr || better(channel, *best, commandCritical)) best = &channel;
        }
        if (best == nullptr) return {ChannelSelectionStatus::NoQualifiedChannel, {}, "NO_QUALIFIED_CHANNEL"};
        return {ChannelSelectionStatus::Selected, best->id, "QUALIFIED_CHANNEL"};
    }

    bool requestFailover(const std::string& activeId, bool commandCritical) const {
        const auto active = find(activeId);
        if (active == nullptr || !qualified(*active, commandCritical)) {
            return selectBest(commandCritical).status == ChannelSelectionStatus::Selected;
        }
        return false;
    }

    std::size_t size() const { return channels_.size(); }

private:
    const ChannelSnapshot* find(const std::string& id) const {
        const auto it = std::find_if(channels_.begin(), channels_.end(),
            [&](const ChannelSnapshot& channel) { return channel.id == id; });
        return it == channels_.end() ? nullptr : &*it;
    }

    static bool qualified(const ChannelSnapshot& channel, bool commandCritical) {
        if (channel.state != ChannelState::Available &&
            channel.state != ChannelState::Active &&
            channel.state != ChannelState::Standby) return false;
        if (!channel.integrityOk || !channel.authenticated) return false;
        if (commandCritical && channel.capacityKbps <= 0.0) return false;
        return true;
    }

    static bool better(const ChannelSnapshot& lhs, const ChannelSnapshot& rhs, bool commandCritical) {
        if (lhs.priority != rhs.priority) return lhs.priority > rhs.priority;
        if (lhs.integrityOk != rhs.integrityOk) return lhs.integrityOk;
        if (lhs.authenticated != rhs.authenticated) return lhs.authenticated;
        if (lhs.latencyMs != rhs.latencyMs) return lhs.latencyMs < rhs.latencyMs;
        if (lhs.packetLoss != rhs.packetLoss) return lhs.packetLoss < rhs.packetLoss;
        if (commandCritical && lhs.capacityKbps != rhs.capacityKbps) return lhs.capacityKbps > rhs.capacityKbps;
        return lhs.id < rhs.id;
    }

    std::vector<ChannelSnapshot> channels_;
};

} // namespace bluesky::c2
