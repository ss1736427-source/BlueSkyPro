#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[3]
VERIFICATION = ROOT / "04_SOFTWARE" / "AUTOPILOT_ADAPTER" / "verification"
sys.path.insert(0, str(VERIFICATION))

from evidence_adapter import AutopilotTelemetrySourceAdapter
from evidence_session import EvidenceSession


class Ph4Int024LogAcquisitionTest(unittest.TestCase):
    def test_log_acquisition_and_provenance_preserve_correlation(self) -> None:
        binary = ROOT / "build" / "autopilot-adapter" / "ph4_int_024_log_acquisition_fixture"
        self.assertTrue(binary.exists(), f"fixture binary missing: {binary}")
        parts = subprocess.check_output([str(binary)], text=True).strip().split("|")
        self.assertEqual(parts[:5], [
            "PH4_INT_024", "FLIGHT-RECORD-PH4-INT-024", "UAV-LOG-024", "MAVLINK2", "BASELINE-1"
        ])
        results = dict(item.split("=", 1) for item in parts[5:])
        for key in (
            "CAPABILITY", "DISCONNECTED_REJECTED", "ACQUIRED", "INVALID_LOG_ID",
            "POST_DISCONNECT_REJECTED", "PROVENANCE", "STATE_BOUNDARY",
        ):
            self.assertEqual(results[key], "PASS")

        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(
                run_dir,
                "TEST-RUN-PH4-INT-024",
                evidence_domain_ids=["EC-03"],
                requirement_ids=["REQ-INT-FLIGHT-LOG-ACQUISITION"],
                test_method_id="TM-PH4-INT-024",
                test_case_id="TC-PH4-INT-024",
                configuration_id="CFG-SIL-LOG-024",
                configuration={
                    "evidence_class": "SIL_FIXTURE_ONLY",
                    "protocol": "MAVLINK2",
                    "protocol_version": "BASELINE-1",
                },
                flight_record_id="FLIGHT-RECORD-PH4-INT-024",
                mission_id="MISSION-024",
            )
            common = {
                "protocol": "MAVLINK2",
                "protocol_version": "BASELINE-1",
                "vehicle_id": "UAV-LOG-024",
                "execution_source": "SIL_FIXTURE",
                "flight_record_id": "FLIGHT-RECORD-PH4-INT-024",
                "mission_id": "MISSION-024",
                "source_id": "SRC-09",
                "log_id": "LOG-024-A",
            }
            evidence = [
                (1, "capability", "flight_log_acquisition", "PASS"),
                (2, "acquisition", "LOG-024-A", "ACQUIRED"),
                (3, "provenance", "SIL_LOG_SOURCE:LOG-024-A", "VERIFIED"),
                (4, "state_gate", "DISCONNECTED", "REJECTED"),
            ]
            for timestamp, parameter, value, result in evidence:
                session.record(
                    AutopilotTelemetrySourceAdapter(),
                    {
                        "timestamp_ms": timestamp,
                        "parameter": parameter,
                        "value": value,
                        "unit": "state",
                        "source": "FLIGHT_LOG_ACQUISITION",
                        "quality": "FIXTURE",
                        "context": {**common, "result": result},
                    },
                )
            session.finalize()

            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            events = [
                json.loads(line)
                for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines()
                if line.strip()
            ]
            self.assertEqual(record["flight_record_id"], "FLIGHT-RECORD-PH4-INT-024")
            self.assertEqual(record["mission_id"], "MISSION-024")
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(len(events), 4)
            for event in events:
                context = event["context"]
                self.assertEqual(context["vehicle_id"], "UAV-LOG-024")
                self.assertEqual(context["flight_record_id"], "FLIGHT-RECORD-PH4-INT-024")
                self.assertEqual(context["mission_id"], "MISSION-024")
                self.assertEqual(context["source_id"], "SRC-09")
                self.assertEqual(context["log_id"], "LOG-024-A")


if __name__ == "__main__":
    unittest.main()
