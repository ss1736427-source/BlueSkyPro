#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent / "verification"
sys.path.insert(0, str(ROOT))
from evidence_adapter import AutopilotTelemetrySourceAdapter, C2LinkSourceAdapter
from evidence_session import EvidenceSession


def main() -> int:
    bridge = Path(sys.argv[1])
    with tempfile.TemporaryDirectory() as tmp:
        raw = Path(tmp) / "source.jsonl"
        result = subprocess.run([str(bridge), str(raw)], capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr
        events = [json.loads(line) for line in raw.read_text(encoding="utf-8").splitlines() if line]
        assert len(events) == 2
        record_ids = {event["context"]["flight_record_id"] for event in events}
        timestamps = {event["timestamp_ms"] for event in events}
        assert record_ids == {"FLIGHT-RECORD-PH4-INT-010"}
        assert len(timestamps) == 1 and next(iter(timestamps)) > 0

        run_dir = Path(tmp) / "RUN-PH4-INT-010"
        session = EvidenceSession(
            run_dir,
            "RUN-PH4-INT-010",
            evidence_domain_ids=["EC-01", "EC-17"],
            requirement_ids=["SYS-INT-001"],
            test_method_id="METHOD-PH4-INT-010",
            test_case_id="CASE-PH4-INT-010",
            configuration_id="CFG-PH4-INT-010",
            configuration={"evidence_class": "DEMONSTRATION"},
            flight_record_id="FLIGHT-RECORD-PH4-INT-010",
            mission_id="MISSION-FIXTURE-010",
            session_id="SES-PH4-INT-010",
        )
        adapters = [AutopilotTelemetrySourceAdapter(), C2LinkSourceAdapter()]
        for event, adapter in zip(events, adapters):
            session.record(adapter, event)
        session.finalize()

        stored = [json.loads(x) for x in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if x]
        assert len(stored) == 2
        assert all(item["context"]["flight_record_id"] == "FLIGHT-RECORD-PH4-INT-010" for item in stored)
        assert all(item["timestamp_ms"] == next(iter(timestamps)) for item in stored)

    print("PH4_INT_010_PRODUCER_TO_EVIDENCE: PASS")
    print("flight_record_id_end_to_end=PASS")
    print("producer_timestamp_end_to_end=PASS")
    print("verification_status=FIXTURE_ONLY")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
