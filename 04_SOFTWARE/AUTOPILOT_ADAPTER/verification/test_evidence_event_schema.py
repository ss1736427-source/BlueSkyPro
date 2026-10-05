from __future__ import annotations

import json
import tempfile
from pathlib import Path

from evidence_capture import EvidenceCapture
from evidence_package_validator import validate_package


def test_real_evidence_capture_event_matches_validator_contract() -> None:
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        capture = EvidenceCapture(root)
        capture.record(
            event_id="EV-001",
            evidence_domain_id="EC-14",
            source_id="SRC-13",
            parameter="time_sync_status",
            value="VALID",
            unit="state",
            quality="VALID",
            test_run_id="TR-001",
            flight_record_id="FR-001",
            configuration_id="CFG-001",
            evidence_class="DEVELOPMENT",
            timestamp_utc="2026-09-14T20:00:00Z",
        )
        events = [json.loads(line) for line in (root / "events.jsonl").read_text(encoding="utf-8").splitlines()]
        assert events[0]["source_id"] == "SRC-13"
        assert events[0]["evidence_domain_id"] == "EC-14"
