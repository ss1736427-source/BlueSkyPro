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


def verify_session(events, run_dir: Path, record_id: str, label: str) -> None:
    session = EvidenceSession(
        run_dir,
        f"RUN-PH4-INT-012-{label}",
        evidence_domain_ids=["EC-01", "EC-17"],
        requirement_ids=["SYS-INT-001"],
        test_method_id="METHOD-PH4-INT-012",
        test_case_id=f"CASE-PH4-INT-012-{label}",
        configuration_id="CFG-PH4-INT-012",
        configuration={"evidence_class": "DEMONSTRATION"},
        flight_record_id=record_id,
        mission_id="MISSION-FIXTURE-012",
        session_id=f"SES-PH4-INT-012-{label}",
    )
    adapter = BlueSkyEventSourceAdapter()
    for event in events:
        session.record(adapter, event)
    session.finalize()
    stored = [json.loads(x) for x in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if x]
    assert len(stored) == len(events)
    assert all(item["context"]["flight_record_id"] == record_id for item in stored)


def main() -> int:
    bridge = Path(sys.argv[1])
    with tempfile.TemporaryDirectory() as tmp:
        raw = Path(tmp) / "source.jsonl"
        result = subprocess.run([str(bridge), str(raw)], capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr
        events = [json.loads(line) for line in raw.read_text(encoding="utf-8").splitlines() if line]
        assert len(events) == 10

        success = [e for e in events if e["context"]["flight_record_id"].endswith("SUCCESS")]
        abort = [e for e in events if e["context"]["flight_record_id"].endswith("ABORT")]
        assert len(success) == 5
        assert len(abort) == 5

        assert success[0]["context"]["lifecycle_state"] == "PREPARING"
        assert success[1]["context"]["lifecycle_state"] == "ACTIVE"
        assert success[2]["parameter"] == "autopilot_start_mission"
        assert success[2]["context"]["acknowledged"] is True
        assert success[3]["parameter"] == "c2_mission_transfer"
        assert success[3]["context"]["acknowledged"] is True
        assert success[4]["context"]["lifecycle_state"] == "COMPLETED"

        assert abort[0]["context"]["lifecycle_state"] == "PREPARING"
        assert abort[1]["context"]["lifecycle_state"] == "ACTIVE"
        assert abort[2]["parameter"] == "c2_mission_transfer"
        assert abort[2]["context"]["acknowledged"] is False
        assert abort[2]["context"]["error"] == "NOT_CONNECTED"
        assert abort[3]["parameter"] == "autopilot_abort_mission"
        assert abort[3]["context"]["acknowledged"] is False
        assert abort[3]["context"]["error"] == "NOT_CONNECTED"
        assert abort[4]["context"]["lifecycle_state"] == "ABORTED"

        verify_session(success, Path(tmp) / "success", "FLIGHT-RECORD-PH4-INT-012-SUCCESS", "SUCCESS")
        verify_session(abort, Path(tmp) / "abort", "FLIGHT-RECORD-PH4-INT-012-ABORT", "ABORT")

    print("PH4_INT_012_OPERATIONAL_EXECUTION_TO_EVIDENCE: PASS")
    print("success_execution_binding=PASS")
    print("failure_abort_binding=PASS")
    print("flight_record_provenance=PASS")
    print("verification_status=FIXTURE_ONLY")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
