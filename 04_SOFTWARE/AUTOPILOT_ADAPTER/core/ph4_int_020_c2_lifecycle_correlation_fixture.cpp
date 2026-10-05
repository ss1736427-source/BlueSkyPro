#include "c2_interface_baseline.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    try {
        using namespace bluesky::c2;
        const std::string flightRecordId = "FLIGHT-RECORD-PH4-INT-020";
        const std::string vehicleId = "UAV-C2-020";
        const std::string channel = "COMMAND";
        const std::string payload = "MISSION=READY";

        C2InterfaceBaseline c2({vehicleId, "LOOPBACK", "BLUESKY-C2", "1.0"});
        require(c2.getLinkState() == C2LinkState::Disconnected, "INITIAL_STATE_FAILED");

        const auto preconnect = c2.send(channel, payload);
        require(!preconnect.accepted, "PRECONNECT_SEND_ACCEPTED");
        require(preconnect.error == C2ErrorCode::NotConnected, "PRECONNECT_ERROR_FAILED");

        const auto badAuth = c2.authenticate("");
        require(!badAuth.accepted, "EMPTY_AUTH_ACCEPTED");
        require(badAuth.error == C2ErrorCode::AuthenticationFailed, "EMPTY_AUTH_ERROR_FAILED");

        require(c2.connect(), "CONNECT_FAILED");
        require(c2.getLinkState() == C2LinkState::Connected, "CONNECTED_STATE_FAILED");

        const auto auth = c2.authenticate("CRED-PH4-INT-020");
        require(auth.accepted, "AUTHENTICATION_FAILED");

        const auto heartbeat = c2.heartbeat(2000);
        require(heartbeat.accepted, "HEARTBEAT_FAILED");
        require(c2.getLinkMetrics().healthy, "HEALTHY_STATE_FAILED");

        const auto tx = c2.send(channel, payload);
        require(tx.accepted, "SEND_FAILED");
        require(tx.sequence != 0, "SEND_SEQUENCE_MISSING");

        const auto rx = c2.receive();
        require(rx.has_value(), "RECEIVE_EMPTY");
        require(rx->sequence == tx.sequence, "SEQUENCE_CORRELATION_FAILED");
        require(rx->channel == channel, "CHANNEL_CORRELATION_FAILED");
        require(rx->payload == payload, "PAYLOAD_CORRELATION_FAILED");

        c2.disconnect();
        require(c2.getLinkState() == C2LinkState::Disconnected, "DISCONNECT_STATE_FAILED");
        require(!c2.getLinkMetrics().healthy, "DISCONNECT_HEALTH_FAILED");

        const auto postdisconnect = c2.heartbeat(3000);
        require(!postdisconnect.accepted, "POSTDISCONNECT_HEARTBEAT_ACCEPTED");
        require(postdisconnect.error == C2ErrorCode::LinkLost, "POSTDISCONNECT_ERROR_FAILED");

        std::cout << "PH4_INT_020|" << flightRecordId << "|" << vehicleId << "|BLUESKY-C2|1.0|"
                  << "PRECONNECT_REJECTED|AUTH_REJECTED|CONNECTED|AUTHENTICATED|HEARTBEAT_OK|"
                  << "TX_RX_MATCH|DISCONNECTED|POSTDISCONNECT_REJECTED|SEQ=" << tx.sequence << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "PH4_INT_020_EXCEPTION|" << error.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "PH4_INT_020_EXCEPTION|UNKNOWN\n";
        return 1;
    }
}
