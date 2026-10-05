#include "flight_evidence_recorder.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {

bool check(bool condition, const char* label) {
    std::cout << (condition ? "PASS: " : "FAIL: ") << label << std::endl;
    return condition;
}

} // namespace

int main() {
    try {
        std::cout << "STEP 1: prepare temp directory" << std::endl;
        const auto root = std::filesystem::temp_directory_path() / "bluesky_ec14_recorder_test";
        std::filesystem::remove_all(root);

        std::cout << "STEP 2: initialize recorder" << std::endl;
        bluesky::verification::FlightEvidenceRecorder recorder(root);
        recorder.init("SES-001", "TR-001", "FR-001", "MIS-001", "CFG-001", "2026-09-14T07:00:00Z");

        std::cout << "STEP 3: collect events" << std::endl;
        recorder.collect({"2026-09-14T07:00:01Z", "SRC-01", "EC-01", "link_state", "connected", "", "RAW-001"});
        recorder.collect({"2026-09-14T07:00:02Z", "SRC-02", "EC-02", "altitude", "120.5", "m", "RAW-002"});

        std::cout << "STEP 4: finalize" << std::endl;
        recorder.finalize("2026-09-14T07:00:03Z");

        if (!check(std::filesystem::exists(root / "record.json"), "record.json exists")) return 1;
        if (!check(std::filesystem::exists(root / "events.jsonl"), "events.jsonl exists")) return 1;

        std::cout << "STEP 5: read original events" << std::endl;
        std::string original;
        {
            std::ifstream raw(root / "events.jsonl", std::ios::binary);
            if (!check(static_cast<bool>(raw), "events.jsonl opens for original read")) return 1;
            original.assign(std::istreambuf_iterator<char>(raw), {});
        }
        if (!check(!original.empty(), "original events are non-empty")) return 1;

        std::cout << "STEP 6: replay" << std::endl;
        bluesky::verification::FlightEvidenceReplay replay(root);
        const auto events = replay.readAll();
        if (!check(events.size() == 2, "replay returns two events")) return 1;
        if (!check(events[0].find("RAW-001") != std::string::npos, "first event contains RAW-001")) return 1;
        if (!check(events[1].find("RAW-002") != std::string::npos, "second event contains RAW-002")) return 1;

        std::cout << "STEP 7: verify raw file unchanged" << std::endl;
        std::string unchanged;
        {
            std::ifstream raw_after(root / "events.jsonl", std::ios::binary);
            if (!check(static_cast<bool>(raw_after), "events.jsonl opens for unchanged read")) return 1;
            unchanged.assign(std::istreambuf_iterator<char>(raw_after), {});
        }
        if (!check(original == unchanged, "events.jsonl is byte-for-byte unchanged")) return 1;

        std::cout << "STEP 8: cleanup" << std::endl;
        std::filesystem::remove_all(root);
        std::cout << "PASS: flight_evidence_recorder_test" << std::endl;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "EXCEPTION: " << error.what() << std::endl;
        return 2;
    } catch (...) {
        std::cerr << "EXCEPTION: unknown" << std::endl;
        return 3;
    }
}
