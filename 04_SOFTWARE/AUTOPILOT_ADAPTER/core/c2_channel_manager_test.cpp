#include "c2_channel_manager.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace bluesky::c2;

    C2ChannelManager manager;
    manager.registerChannel({"PRIMARY", "RF", ChannelState::Active, 40.0, 0.01, 1000.0, true, true, 100, 1000});
    manager.registerChannel({"BACKUP", "CELLULAR", ChannelState::Standby, 80.0, 0.02, 500.0, true, true, 50, 900});

    const auto selected = manager.selectBest(true);
    assert(selected.status == ChannelSelectionStatus::Selected);
    assert(selected.channelId == "PRIMARY");

    manager.update({"PRIMARY", "RF", ChannelState::Degraded, 400.0, 0.30, 1000.0, true, true, 100, 1100});
    assert(manager.requestFailover("PRIMARY", true));
    const auto backup = manager.selectBest(true);
    assert(backup.status == ChannelSelectionStatus::Selected);
    assert(backup.channelId == "BACKUP");

    manager.update({"BACKUP", "CELLULAR", ChannelState::Unavailable, 0.0, 1.0, 0.0, true, true, 50, 1200});
    const auto none = manager.selectBest(true);
    assert(none.status == ChannelSelectionStatus::NoQualifiedChannel);

    manager.update({"PRIMARY", "RF", ChannelState::Standby, 45.0, 0.01, 1000.0, true, true, 100, 1300});
    assert(manager.selectBest(false).channelId == "PRIMARY");

    manager.update({"PRIMARY", "RF", ChannelState::Active, 45.0, 0.01, 1000.0, false, true, 100, 1400});
    assert(manager.selectBest(false).status == ChannelSelectionStatus::NoQualifiedChannel);

    std::cout << "ARCH-OPS-016 C2 channel manager: PASS\n";
    return 0;
}
