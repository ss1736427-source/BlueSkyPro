#!/usr/bin/env python3
"""Deterministic PH4-INT-006 bridge from integration results to EvidenceSession."""
from __future__ import annotations

import json
import tempfile
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent.parent / "verification"
sys.path.insert(0, str(ROOT))

from evidence_adapter import SourceAdapter
from evidence_session import EvidenceSession


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        run_dir = Path(tmp) / "RUN-PH4-INT-006"
        session = EvidenceSession(
            run_dir,
            "RUN-PH4-INT-006",
            evidence_domain_ids=["EC-01"],
            configuration_id="CFG-PH4-INT-006",
            mission_id="MISSION-FIXTURE-006",
            session_id="SES-PH4-INT-006",
            data_class="DEMONSTRATION",
        )
        adapter = SourceAdapter("AUTOPILOT_ADAPTER")
        session.record(adapter, {
            "timestamp_ms": 1000,
            "parameter": "command_result",
            "value": "REJECTED",
            "unit": "none",
            "quality": "VALID",
        })
        session.record(adapter, {
            "timestamp_ms": 1001,
            "parameter": "error_reason",
            "value": "AUTOPILOT_NOT_CONNECTED",
            "unit": "none",
            "quality": "VALID",
        })
        session.record(adapter, {
            "timestamp_ms": 1002,
            "parameter": "readback_comparison",
            "value": "MISSION_CONTENT_MISMATCH",
            "unit": "none",
            "quality": "VALID",
        })
        session.finalize()

        record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
        assert record["lifecycle_state"] == "ARCHIVE"
        assert record["session_id"] == "SES-PH4-INT-006"
        assert record["test_run_id"] == "RUN-PH4-INT-006"
        assert record["mission_id"] == "MISSION-FIXTURE-006"
        assert (run_dir / "events.jsonl").exists()
        assert (run_dir / "manifest.json").exists()
        assert (run_dir / "manifest.sha256").exists()
        assert (run_dir / "report.md").exists()

        events = [json.loads(line) for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if line]
        assert len(events) == 3
        for event in events:
            assert event["timestamp_ms"] > 0
            assert event["parameter"]
            assert event["source"] == "AUTOPILOT_ADAPTER"
            assert event["context"]["session_id"] == "SES-PH4-INT-006"
            assert event["context"]["test_run_id"] == "RUN-PH4-INT-006"
            assert event["context"]["evidence_domain_id"] == "EC-01"

    print("PH4_INT_006_EVIDENCE_SESSION_CHAIN: PASS")
    print("integration_result_to_evidence_session=PASS")
    print("provenance_preserved=PASS")
    print("failure_and_mismatch_events_preserved=PASS")
    print("final_package_validation=PASS")
    print("verification_status=FIXTURE_ONLY")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
