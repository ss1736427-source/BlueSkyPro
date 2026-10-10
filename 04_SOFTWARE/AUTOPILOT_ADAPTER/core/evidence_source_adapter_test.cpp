#include "evidence_source_adapter.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using namespace bluesky::verification;

static bool check(bool condition, const char* label) {
    if (!condition) {
        std::cerr << "FAIL: " << label << '\n';
        return false;
    }
    std::cout << "PASS: " << label << '\n';
    return true;
}

int main() {
    try {
        std::cout << "STEP 1: prepare temp directory\n";
        const auto directory = std::filesystem::temp_directory_path() / "bluesky_source_adapter_test";
        std::filesystem::remove_all(directory);

        std::cout << "STEP 2: initialize recorder\n";
        FlightEvidenceRecorder recorder(directory);
        recorder.init("SESSION-SA-001", "TEST-SA-001", "FLIGHT-SA-001",
                      "MISSION-SA-001", "CONFIG-SA-001", "2026-09-14T16:00:00Z");

        std::cout << "STEP 3: create source record\n";
        EvidenceSourceAdapter adapter;
        const SourceRecord source{
            "2026-09-14T16:00:01Z",
            "SRC-01",
            "EC-01",
            "position.latitude",
            "60.1699",
            "deg",
            "RAW-SOURCE-001"
        };

        std::cout << "STEP 4: adapt source record\n";
        const EvidenceEvent event = adapter.adapt(source);
        if (!check(event.timestamp_utc == source.timestamp_utc, "timestamp")) return 1;
        if (!check(event.source_id == source.source_id, "source_id")) return 1;
        if (!check(event.evidence_domain_id == source.evidence_domain_id, "evidence_domain_id")) return 1;
        if (!check(event.parameter == source.parameter, "parameter")) return 1;
        if (!check(event.value == source.value, "value")) return 1;
        if (!check(event.unit == source.unit, "unit")) return 1;
        if (!check(event.raw_record_reference == source.raw_record_reference, "raw_record_reference")) return 1;

        std::cout << "STEP 5: collect adapted event\n";
        recorder.collect(event);

        std::cout << "STEP 6: finalize recorder\n";
        recorder.finalize("2026-09-14T16:00:02Z");

        std::cout << "STEP 7: verify events.jsonl\n";
        std::ifstream in(directory / "events.jsonl", std::ios::binary);
        if (!check(static_cast<bool>(in), "events.jsonl opened")) return 1;
        std::string line;
        if (!check(static_cast<bool>(std::getline(in, line)), "events.jsonl contains a line")) return 1;
        if (!check(line.find("SRC-01") != std::string::npos, "SRC-01 persisted")) return 1;
        if (!check(line.find("EC-01") != std::string::npos, "EC-01 persisted")) return 1;
        if (!check(line.find("RAW-SOURCE-001") != std::string::npos, "raw reference persisted")) return 1;

        std::cout << "STEP 8: cleanup\n";
        in.close();
        std::filesystem::remove_all(directory);
        std::cout << "PASS: final\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "EXCEPTION: " << ex.what() << '\n';
        return 2;
    } catch (...) {
        std::cerr << "EXCEPTION: unknown\n";
        return 3;
    }
}
