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


class Ph4Int023ContingencyCommandTest(unittest.TestCase):
    def test_contingency_commands_and_negative_paths_preserve_correlation(self) -> None:
        binary = ROOT / "build" / "autopilot-adapter" / "ph4_int_023_contingency_command_fixture"
        self.assertTrue(binary.exists(), f"fixture binary missing: {binary}")
        parts = subprocess.check_output([str(binary)], text=True).strip().split("|")
        self.assertEqual(parts[:5], [
            "PH4_INT_023", "FLIGHT-RECORD-PH4-INT-023", "UAV-CONTINGENCY-023", "MAVLINK2", "BASELINE-1"
        ])
        results = dict(item.split("=", 1) for item in parts[5:])
        for key in (
            "HOLD", "RTL", "LAND", "ABORT", "UNSUPPORTED", "INVALID_STATE",
            "SAFETY_REJECTED", "LINK_LOST", "STATE_BOUNDARY",
        ):
            self.assertEqual(results[key], "PASS")

        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(
                run_dir,
                "TEST-RUN-PH4-INT-023",
                evidence_domain_ids=["EC-03"],
                requirement_ids=["REQ-INT-CONTINGENCY-COMMAND-BEHAVIOR"],
                test_method_id="TM-PH4-INT-023",
                test_case_id="TC-PH4-INT-023",
                configuration_id="CFG-SIL-CONTINGENCY-023",
                configuration={
                    "evidence_class": "SIL_FIXTURE_ONLY",
                    "protocol": "MAVLINK2",
                    "protocol_version": "BASELINE-1",
                },
                flight_record_id="FLIGHT-RECORD-PH4-INT-023",
                mission_id="MISSION-023",
            )
            common = {
                "protocol": "MAVLINK2",
                "protocol_version": "BASELINE-1",
                "vehicle_id": "UAV-CONTINGENCY-023",
                "execution_source": "SIL_FIXTURE",
                "flight_record_id": "FLIGHT-RECORD-PH4-INT-023",
                "mission_id": "MISSION-023",
                "source_id": "SRC-08",
            }
            evidence = [
                (1, "contingency_command", "HOLD", "ACK"),
                (2, "contingency_command", "RTL", "ACK"),
                (3, "contingency_command", "LAND", "ACK"),
                (4, "contingency_command", "ABORT", "ACK"),
                (5, "contingency_command", "UNSUPPORTED", "NACK"),
                (6, "contingency_command", "INVALID_STATE", "NACK"),
                (7, "contingency_command", "SAFETY_REJECTED", "NACK"),
                (8, "contingency_command", "LINK_LOST", "NACK"),
            ]
            for timestamp, value, correlation, acknowledgement in evidence:
                session.record(
                    AutopilotTelemetrySourceAdapter(),
                    {
                        "timestamp_ms": timestamp,
                        "parameter": value,
                        "value": acknowledgement,
                        "unit": "state",
                        "source": "CONTINGENCY_COMMAND",
                        "quality": "FIXTURE",
                        "context": {**common, "correlation": correlation},
                    },
                )
            session.finalize()
            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            events = [json.loads(line) for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if line.strip()]
            self.assertEqual(record["flight_record_id"], "FLIGHT-RECORD-PH4-INT-023")
            self.assertEqual(record["mission_id"], "MISSION-023")
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(len(events), 8)
            for event in events:
                context = event["context"]
                self.assertEqual(context["vehicle_id"], "UAV-CONTINGENCY-023")
                self.assertEqual(context["flight_record_id"], "FLIGHT-RECORD-PH4-INT-023")
                self.assertEqual(context["mission_id"], "MISSION-023")
                self.assertEqual(context["source_id"], "SRC-08")


if __name__ == "__main__":
    unittest.main()
