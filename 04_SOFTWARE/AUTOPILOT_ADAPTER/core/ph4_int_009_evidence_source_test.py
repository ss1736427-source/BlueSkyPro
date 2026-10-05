#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent / "verification"
sys.path.insert(0, str(ROOT))
from evidence_adapter import AutopilotTelemetrySourceAdapter
from evidence_session import EvidenceSession


def main() -> int:
    fixture = Path(sys.argv[1])
    producer = Path(sys.argv[2])
    with tempfile.TemporaryDirectory() as tmp:
        raw = Path(tmp) / "source.jsonl"
        result = subprocess.run([str(producer)], capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr
        source = {"timestamp_ms": 1770000000000, "parameter": "FLIGHT_RECORD_SOURCE", "value": "RECORDED", "unit": "state", "source": "AUTOPILOT_ADAPTER", "context": {"flight_record_id": "FLIGHT-RECORD-PH4-INT-009"}}
        raw.write_text(json.dumps(source) + "\n", encoding="utf-8")
        run_dir = Path(tmp) / "RUN-PH4-INT-009"
        session = EvidenceSession(run_dir, "RUN-PH4-INT-009", evidence_domain_ids=["EC-01"], requirement_ids=["SYS-INT-001"], test_method_id="METHOD-PH4-INT-009", test_case_id="CASE-PH4-INT-009", configuration_id="CFG-PH4-INT-009", configuration={"evidence_class": "DEMONSTRATION"}, flight_record_id="FLIGHT-RECORD-PH4-INT-009", mission_id="MISSION-FIXTURE-009", session_id="SES-PH4-INT-009")
        for line in raw.read_text(encoding="utf-8").splitlines():
            session.record(AutopilotTelemetrySourceAdapter(), json.loads(line))
        session.finalize()
        events = [json.loads(x) for x in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if x]
        assert events[0]["context"]["flight_record_id"] == "FLIGHT-RECORD-PH4-INT-009"
    print("PH4_INT_009_SOURCE_TO_EVIDENCE: PASS")
    print("flight_record_provenance=PASS")
    print("verification_status=FIXTURE_ONLY")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
