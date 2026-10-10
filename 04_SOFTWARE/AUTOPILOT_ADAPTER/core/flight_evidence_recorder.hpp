#pragma once

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace bluesky::verification {

struct EvidenceEvent {
    std::string timestamp_utc;
    std::string source_id;
    std::string evidence_domain_id;
    std::string parameter;
    std::string value;
    std::string unit;
    std::string raw_record_reference;
};

class FlightEvidenceRecorder final {
public:
    explicit FlightEvidenceRecorder(std::filesystem::path session_directory)
        : directory_(std::move(session_directory)) {}

    void init(const std::string& session_id,
              const std::string& test_run_id,
              const std::string& flight_record_id,
              const std::string& mission_id,
              const std::string& configuration_id,
              const std::string& started_at_utc) {
        if (initialized_) throw std::logic_error("session already initialized");
        std::filesystem::create_directories(directory_);
        session_id_ = session_id;
        test_run_id_ = test_run_id;
        flight_record_id_ = flight_record_id;
        mission_id_ = mission_id;
        configuration_id_ = configuration_id;
        started_at_utc_ = started_at_utc;
        initialized_ = true;
        state_ = "INIT";
        writeRecord();
    }

    void collect(const EvidenceEvent& event) {
        requireInitialized();
        if (state_ == "INIT") state_ = "COLLECT";
        if (state_ != "COLLECT") throw std::logic_error("session is not collecting");
        std::ofstream out(directory_ / "events.jsonl", std::ios::app | std::ios::binary);
        if (!out) throw std::runtime_error("cannot open events.jsonl");
        out << "{\"session_id\":\"" << escape(session_id_)
            << "\",\"timestamp_utc\":\"" << escape(event.timestamp_utc)
            << "\",\"source_id\":\"" << escape(event.source_id)
            << "\",\"evidence_domain_id\":\"" << escape(event.evidence_domain_id)
            << "\",\"parameter\":\"" << escape(event.parameter)
            << "\",\"value\":\"" << escape(event.value)
            << "\",\"unit\":\"" << escape(event.unit)
            << "\",\"raw_record_reference\":\"" << escape(event.raw_record_reference)
            << "\"}\n";
    }

    void finalize(const std::string& ended_at_utc) {
        requireInitialized();
        if (state_ != "COLLECT" && state_ != "INIT") throw std::logic_error("invalid finalize state");
        ended_at_utc_ = ended_at_utc;
        state_ = "FINALIZE";
        writeRecord();
    }

    const std::filesystem::path& directory() const noexcept { return directory_; }

private:
    static std::string escape(const std::string& value) {
        std::string result;
        result.reserve(value.size());
        for (const char c : value) {
            if (c == '\\') result += "\\\\";
            else if (c == '"') result += "\\\"";
            else if (c == '\n') result += "\\n";
            else result += c;
        }
        return result;
    }

    void requireInitialized() const {
        if (!initialized_) throw std::logic_error("session is not initialized");
    }

    void writeRecord() const {
        std::ofstream out(directory_ / "record.json", std::ios::binary | std::ios::trunc);
        if (!out) throw std::runtime_error("cannot open record.json");
        out << "{\n"
            << "  \"session_id\": \"" << escape(session_id_) << "\",\n"
            << "  \"test_run_id\": \"" << escape(test_run_id_) << "\",\n"
            << "  \"flight_record_id\": \"" << escape(flight_record_id_) << "\",\n"
            << "  \"mission_id\": \"" << escape(mission_id_) << "\",\n"
            << "  \"configuration_id\": \"" << escape(configuration_id_) << "\",\n"
            << "  \"started_at_utc\": \"" << escape(started_at_utc_) << "\",\n"
            << "  \"ended_at_utc\": \"" << escape(ended_at_utc_) << "\",\n"
            << "  \"state\": \"" << state_ << "\"\n"
            << "}\n";
    }

    std::filesystem::path directory_;
    std::string session_id_, test_run_id_, flight_record_id_, mission_id_, configuration_id_;
    std::string started_at_utc_, ended_at_utc_;
    std::string state_ = "UNINITIALIZED";
    bool initialized_ = false;
};

class FlightEvidenceReplay final {
public:
    explicit FlightEvidenceReplay(std::filesystem::path session_directory)
        : events_(session_directory / "events.jsonl") {}

    std::vector<std::string> readAll() const {
        std::ifstream in(events_, std::ios::binary);
        if (!in) throw std::runtime_error("cannot open events.jsonl");
        std::vector<std::string> result;
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty()) result.push_back(line);
        }
        return result;
    }

private:
    std::filesystem::path events_;
};

} // namespace bluesky::verification
