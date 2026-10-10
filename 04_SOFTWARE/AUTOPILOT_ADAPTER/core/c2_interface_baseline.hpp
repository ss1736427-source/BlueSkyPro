#pragma once

#include "universal_c2_interface.hpp"

#include <deque>

namespace bluesky::c2 {

class C2InterfaceBaseline final : public UniversalC2Interface {
public:
    explicit C2InterfaceBaseline(C2Identity identity);

    C2Identity getIdentity() const override;
    bool connect() override;
    void disconnect() override;
    C2LinkState getLinkState() const override;
    C2LinkMetrics getLinkMetrics() const override;

    C2Result send(const std::string& channel, const std::string& payload) override;
    std::optional<C2Frame> receive() override;
    C2Result heartbeat(std::int64_t timestampMs) override;
    C2Result authenticate(const std::string& credentialReference) override;

private:
    C2Identity identity_;
    C2LinkState state_{C2LinkState::Disconnected};
    C2LinkMetrics metrics_{};
    std::deque<C2Frame> rxQueue_;
    std::uint64_t nextSequence_{0};
    bool authenticated_{false};
};

} // namespace bluesky::c2
