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


def check(events, run_dir: Path, record_id: str, vehicle_type: str) -> None:
    session = EvidenceSession(
        run_dir,
        f"RUN-PH4-INT-013-{vehicle_type}",
        evidence_domain_ids=["EC-01"],
        requirement_ids=["SYS-INT-001"],
        test_method_id="METHOD-PH4-INT-013",
        test_case_id=f"CASE-PH4-INT-013-{vehicle_type}",
        configuration_id="CFG-PH4-INT-013",
        configuration={"evidence_class": "DEMONSTRATION"},
        flight_record_id=record_id,
        mission_id="MISSION-FIXTURE-013",
        session_id=f"SES-PH4-INT-013-{vehicle_type}",
    )
    adapter = BlueSkyEventSourceAdapter()
    for event in events:
        session.record(adapter, event)
    session.finalize()
    stored = [json.loads(x) for x in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if x]
    assert len(stored) == 1
    assert stored[0]["context"]["flight_record_id"] == record_id
    assert stored[0]["context"]["vehicle_type"] == vehicle_type


def main() -> int:
    bridge = Path(sys.argv[1])
    with tempfile.TemporaryDirectory() as tmp:
        raw = Path(tmp) / "source.jsonl"
        result = subprocess.run([str(bridge), str(raw)], capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr
        events = [json.loads(line) for line in raw.read_text(encoding="utf-8").splitlines() if line]
        assert len(events) == 2
        assert {e["context"]["vehicle_type"] for e in events} == {"MULTIROTOR", "FIXED_WING"}
        assert all(e["context"]["execution_state"] == "ACKNOWLEDGED" for e in events)
        check([events[0]], Path(tmp) / "mr", "FLIGHT-RECORD-PH4-INT-013-MR", "MULTIROTOR")
        check([events[1]], Path(tmp) / "fw", "FLIGHT-RECORD-PH4-INT-013-FW", "FIXED_WING")

    print("PH4_INT_013_HETEROGENEOUS_TO_EVIDENCE: PASS")
    print("multirotor_universal_contract=PASS")
    print("fixed_wing_universal_contract=PASS")
    print("flight_record_provenance=PASS")
    print("verification_status=FIXTURE_ONLY")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
