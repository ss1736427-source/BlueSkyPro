#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
VERIFICATION = ROOT / "04_SOFTWARE" / "AUTOPILOT_ADAPTER" / "verification"
import sys
sys.path.insert(0, str(VERIFICATION))

from evidence_adapter import AutopilotTelemetrySourceAdapter
from evidence_session import EvidenceSession


class Ph4Int022LinkLossResynchronizationTest(unittest.TestCase):
    def test_link_loss_and_resynchronization_preserve_correlation(self) -> None:
        binary = ROOT / "build" / "autopilot-adapter" / "ph4_int_022_link_loss_resynchronization_fixture"
        self.assertTrue(binary.exists(), f"fixture binary missing: {binary}")
        parts = subprocess.check_output([str(binary)], text=True).strip().split("|")
        self.assertEqual(parts[:5], [
            "PH4_INT_022", "FLIGHT-RECORD-PH4-INT-022", "UAV-LINK-022", "MAVLINK2", "BASELINE-1"
        ])
        results = dict(item.split("=", 1) for item in parts[5:])
        for key in ("CONNECTED", "LOSS_DETECTED", "RECOVERING", "MISMATCH_REJECTED", "RESYNCHRONIZED", "RESTORED"):
            self.assertEqual(results[key], "PASS")
        self.assertEqual(results["STATE"], "CONNECTED")

        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(
                run_dir,
                "TEST-RUN-PH4-INT-022",
                evidence_domain_ids=["EC-03"],
                requirement_ids=["REQ-INT-LINK-LOSS-RESYNCHRONIZATION"],
                test_method_id="TM-PH4-INT-022",
                test_case_id="TC-PH4-INT-022",
                configuration_id="CFG-SIL-LINK-022",
                configuration={
                    "evidence_class": "SIL_FIXTURE_ONLY",
                    "protocol": "MAVLINK2",
                    "protocol_version": "BASELINE-1",
                },
                flight_record_id="FLIGHT-RECORD-PH4-INT-022",
                mission_id="MISSION-022",
            )
            common = {
                "protocol": "MAVLINK2",
                "protocol_version": "BASELINE-1",
                "vehicle_id": "UAV-LINK-022",
                "execution_source": "SIL_FIXTURE",
                "flight_record_id": "FLIGHT-RECORD-PH4-INT-022",
                "mission_id": "MISSION-022",
                "source_id": "SRC-07",
            }
            evidence = [
                (1, "connection_state", "CONNECTED", "INITIAL_LINK"),
                (2, "link_loss", "DEGRADED", "LOSS_DETECTED"),
                (3, "telemetry_state", "STALE", "TELEMETRY_INVALIDATED"),
                (4, "recovery_state", "RECOVERING", "RECONNECT_STARTED"),
                (5, "resynchronization_guard", "REJECTED", "MISSION_MISMATCH"),
                (6, "resynchronization", "CONNECTED", "STATE_RESTORED"),
                (7, "telemetry_state", "FRESH", "TELEMETRY_RESTORED"),
                (8, "mission_state", "SYNCHRONIZED", "MISSION_RECONCILED"),
            ]
            for timestamp, parameter, value, correlation in evidence:
                session.record(
                    AutopilotTelemetrySourceAdapter(),
                    {
                        "timestamp_ms": timestamp,
                        "parameter": parameter,
                        "value": value,
                        "unit": "state",
                        "source": "LINK_LIFECYCLE",
                        "quality": "FIXTURE",
                        "context": {**common, "correlation": correlation},
                    },
                )
            session.finalize()
            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            events = [json.loads(line) for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if line.strip()]
            self.assertEqual(record["flight_record_id"], "FLIGHT-RECORD-PH4-INT-022")
            self.assertEqual(record["mission_id"], "MISSION-022")
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(len(events), 8)
            for event in events:
                context = event["context"]
                self.assertEqual(context["vehicle_id"], "UAV-LINK-022")
                self.assertEqual(context["flight_record_id"], "FLIGHT-RECORD-PH4-INT-022")
                self.assertEqual(context["mission_id"], "MISSION-022")
                self.assertEqual(context["source_id"], "SRC-07")


if __name__ == "__main__":
    unittest.main()
