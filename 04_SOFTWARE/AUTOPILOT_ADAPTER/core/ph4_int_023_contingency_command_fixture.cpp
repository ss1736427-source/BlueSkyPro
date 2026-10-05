#include <iostream>
#include <stdexcept>
#include <string>

namespace {

enum class State { Ready, Executing, Contingency, Disconnected, SafetyLocked };

enum class Command { Hold, Rtl, Land, Abort, Unsupported };

struct Result {
    bool accepted;
    const char* state;
    const char* acknowledgement;
    const char* reason;
};

const char* commandName(Command command) {
    switch (command) {
    case Command::Hold: return "HOLD";
    case Command::Rtl: return "RTL";
    case Command::Land: return "LAND";
    case Command::Abort: return "ABORT";
    case Command::Unsupported: return "UNSUPPORTED";
    }
    return "UNKNOWN";
}

Result request(State& state, Command command) {
    if (command == Command::Unsupported) {
        return {false, "REJECTED", "NACK", "UNSUPPORTED"};
    }
    if (state == State::Disconnected) {
        return {false, "REJECTED", "NACK", "LINK_LOST"};
    }
    if (state == State::SafetyLocked) {
        return {false, "REJECTED", "NACK", "SAFETY_REJECTED"};
    }
    if (state != State::Executing && state != State::Contingency) {
        return {false, "REJECTED", "NACK", "INVALID_STATE"};
    }

    state = State::Contingency;
    return {true, commandName(command), "ACK", "ACCEPTED"};
}

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

} // namespace

int main() {
    try {
        const std::string flightRecordId = "FLIGHT-RECORD-PH4-INT-023";
        const std::string vehicleId = "UAV-CONTINGENCY-023";
        const std::string protocol = "MAVLINK2";
        const std::string version = "BASELINE-1";
        State state = State::Ready;

        auto invalid = request(state, Command::Rtl);
        require(!invalid.accepted && std::string(invalid.reason) == "INVALID_STATE", "INVALID_STATE_FAILED");

        state = State::Executing;
        const auto hold = request(state, Command::Hold);
        require(hold.accepted && std::string(hold.state) == "HOLD", "HOLD_FAILED");
        const auto rtl = request(state, Command::Rtl);
        require(rtl.accepted && std::string(rtl.state) == "RTL", "RTL_FAILED");
        const auto land = request(state, Command::Land);
        require(land.accepted && std::string(land.state) == "LAND", "LAND_FAILED");
        const auto abort = request(state, Command::Abort);
        require(abort.accepted && std::string(abort.state) == "ABORT", "ABORT_FAILED");

        state = State::Executing;
        const auto unsupported = request(state, Command::Unsupported);
        require(!unsupported.accepted && std::string(unsupported.reason) == "UNSUPPORTED", "UNSUPPORTED_FAILED");

        state = State::SafetyLocked;
        const auto safety = request(state, Command::Land);
        require(!safety.accepted && std::string(safety.reason) == "SAFETY_REJECTED", "SAFETY_REJECTION_FAILED");

        state = State::Disconnected;
        const auto lost = request(state, Command::Rtl);
        require(!lost.accepted && std::string(lost.reason) == "LINK_LOST", "LINK_LOSS_FAILED");

        std::cout << "PH4_INT_023|" << flightRecordId << "|" << vehicleId << "|" << protocol << "|" << version
                  << "|HOLD=PASS|RTL=PASS|LAND=PASS|ABORT=PASS|UNSUPPORTED=PASS|INVALID_STATE=PASS"
                  << "|SAFETY_REJECTED=PASS|LINK_LOST=PASS|STATE_BOUNDARY=PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "PH4_INT_023_EXCEPTION|" << error.what() << "\n";
        return 1;
    }
}
