#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent / "verification"
sys.path.insert(0, str(ROOT))
from evidence_session import EvidenceSession
from evidence_adapter import BlueSkyEventSourceAdapter


def main(executable: str) -> int:
    with tempfile.TemporaryDirectory() as tmp:
        run_dir = Path(tmp) / "evidence"
        output = Path(tmp) / "events.jsonl"
        result = subprocess.run([executable, str(output)], capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr
        events = [json.loads(line) for line in output.read_text(encoding="utf-8").splitlines() if line.strip()]
        assert len(events) == 1
        event = events[0]
        assert event["context"]["flight_record_id"] == "FLIGHT-RECORD-PH4-INT-014"
        assert event["context"]["vehicle_id"] == "UAV-SIL-014"
        assert event["context"]["execution_source"] == "SIL_FIXTURE"

        session = EvidenceSession(
            run_dir,
            "RUN-PH4-INT-014-SIL",
            evidence_domain_ids=["EC-01"],
            requirement_ids=["SYS-INT-001"],
            test_method_id="TM-PH4-INT-014",
            test_case_id="TC-PH4-INT-014",
            configuration_id="CFG-PH4-INT-014-SIL",
            configuration={"evidence_class": "SIL_FIXTURE_ONLY"},
            flight_record_id="FLIGHT-RECORD-PH4-INT-014",
            mission_id="MISSION-PH4-INT-014",
        )
        session.record(BlueSkyEventSourceAdapter(), event)
        session.finalize()
        stored = [json.loads(line) for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if line]
        assert len(stored) == 1
        assert stored[0]["context"]["flight_record_id"] == "FLIGHT-RECORD-PH4-INT-014"
        assert stored[0]["context"]["execution_source"] == "SIL_FIXTURE"

    print("PH4_INT_014_EVIDENCE_SESSION: PASS")
    print("sil_execution_to_evidence=PASS")
    print("verification_status=SIL_FIXTURE_ONLY")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1]))
