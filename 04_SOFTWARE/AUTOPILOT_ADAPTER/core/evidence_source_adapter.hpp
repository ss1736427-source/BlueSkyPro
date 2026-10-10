#pragma once

#include "flight_evidence_recorder.hpp"

#include <stdexcept>
#include <string>

namespace bluesky::verification {

// Source Adapter converts one source-specific record into the common
// EvidenceEvent. It does not alter or own the original source record.
struct SourceRecord {
    std::string timestamp_utc;
    std::string source_id;
    std::string evidence_domain_id;
    std::string parameter;
    std::string value;
    std::string unit;
    std::string raw_record_reference;
};

class EvidenceSourceAdapter final {
public:
    EvidenceEvent adapt(const SourceRecord& source_record) const {
        if (source_record.timestamp_utc.empty())
            throw std::invalid_argument("source record timestamp is required");
        if (source_record.source_id.empty())
            throw std::invalid_argument("source record source_id is required");
        if (source_record.evidence_domain_id.empty())
            throw std::invalid_argument("source record evidence_domain_id is required");
        if (source_record.parameter.empty())
            throw std::invalid_argument("source record parameter is required");
        if (source_record.raw_record_reference.empty())
            throw std::invalid_argument("source record raw_record_reference is required");

        return EvidenceEvent{
            source_record.timestamp_utc,
            source_record.source_id,
            source_record.evidence_domain_id,
            source_record.parameter,
            source_record.value,
            source_record.unit,
            source_record.raw_record_reference
        };
    }
};

} // namespace bluesky::verification
