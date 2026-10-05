#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent / "verification"
sys.path.insert(0, str(ROOT))
from evidence_adapter import BlueSkyEventSourceAdapter
from evidence_session import EvidenceSession


def main() -> int:
    bridge = Path(sys.argv[1])
    with tempfile.TemporaryDirectory() as tmp:
        raw = Path(tmp) / "source.jsonl"
        result = subprocess.run([str(bridge), str(raw)], capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr
        events = [json.loads(line) for line in raw.read_text(encoding="utf-8").splitlines() if line]
        assert len(events) == 3
        record_ids = {event["context"]["flight_record_id"] for event in events}
        assert record_ids == {"FLIGHT-RECORD-PH4-INT-011"}
        assert [event["context"]["lifecycle_state"] for event in events] == [
            "PREPARING", "ACTIVE", "COMPLETED"
        ]
        assert [event["context"]["event_type"] for event in events] == [
            "MISSION_PREPARATION_STARTED",
            "MISSION_EXECUTION_STARTED",
            "MISSION_EXECUTION_COMPLETED",
        ]

        run_dir = Path(tmp) / "RUN-PH4-INT-011"
        session = EvidenceSession(
            run_dir,
            "RUN-PH4-INT-011",
            evidence_domain_ids=["EC-01"],
            requirement_ids=["SYS-INT-001"],
            test_method_id="METHOD-PH4-INT-011",
            test_case_id="CASE-PH4-INT-011",
            configuration_id="CFG-PH4-INT-011",
            configuration={"evidence_class": "DEMONSTRATION"},
            flight_record_id="FLIGHT-RECORD-PH4-INT-011",
            mission_id="MISSION-FIXTURE-011",
            session_id="SES-PH4-INT-011",
        )
        adapter = BlueSkyEventSourceAdapter()
        for event in events:
            session.record(adapter, event)
        session.finalize()

        stored = [json.loads(x) for x in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if x]
        assert len(stored) == 3
        assert all(item["context"]["flight_record_id"] == "FLIGHT-RECORD-PH4-INT-011" for item in stored)
        assert stored[-1]["context"]["lifecycle_state"] == "COMPLETED"

    print("PH4_INT_011_LIFECYCLE_TO_EVIDENCE: PASS")
    print("mission_execution_lifecycle=PASS")
    print("flight_record_binding_end_to_end=PASS")
    print("verification_status=FIXTURE_ONLY")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
