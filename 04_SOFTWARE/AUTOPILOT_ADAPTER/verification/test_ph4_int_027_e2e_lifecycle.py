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


class Ph4Int027EndToEndLifecycleTest(unittest.TestCase):
    def test_end_to_end_adapter_lifecycle(self) -> None:
        binary = ROOT / "build" / "autopilot-adapter" / "ph4_int_027_e2e_lifecycle_fixture"
        self.assertTrue(binary.exists(), f"fixture binary missing: {binary}")

        parts = subprocess.check_output([str(binary)], text=True).strip().split("|")
        self.assertEqual(parts[:5], ["PH4_INT_027", "FLIGHT-RECORD-PH4-INT-027", "UAV-E2E-027", "MAVLINK2", "BASELINE-1"])
        results = dict(item.split("=", 1) for item in parts[5:])
        for key in (
            "CAPABILITY", "CONNECTED", "IDENTIFIED", "COMPILED", "UPLOADED",
            "READ_BACK", "SEMANTIC_VERIFY", "EXECUTION", "FLIGHT_STATE", "LOG", "CLOSED"
        ):
            self.assertEqual(results[key], "PASS")

        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(
                run_dir,
                "TEST-RUN-PH4-INT-027",
                evidence_domain_ids=["EC-03"],
                requirement_ids=["REQ-INT-E2E-ADAPTER-LIFECYCLE"],
                test_method_id="TM-PH4-INT-027",
                test_case_id="TC-PH4-INT-027",
                configuration_id="CFG-SIL-E2E-027",
                configuration={
                    "evidence_class": "SIL_FIXTURE_ONLY",
                    "protocol": "MAVLINK2",
                    "protocol_version": "BASELINE-1",
                },
                flight_record_id="FLIGHT-RECORD-PH4-INT-027",
                mission_id="MISSION-027",
            )
            common = {
                "protocol": "MAVLINK2",
                "protocol_version": "BASELINE-1",
                "vehicle_id": "UAV-E2E-027",
                "execution_source": "SIL_FIXTURE",
                "flight_record_id": "FLIGHT-RECORD-PH4-INT-027",
                "mission_id": "MISSION-027",
                "source_id": "SRC-11",
            }
            evidence = [
                (1, "connection", "CONNECTED"),
                (2, "identity", "IDENTIFIED"),
                (3, "mission_compile", "COMPILED"),
                (4, "mission_upload", "UPLOADED"),
                (5, "mission_read_back", "READ_BACK"),
                (6, "semantic_verify", "VERIFIED"),
                (7, "execution", "ACKNOWLEDGED"),
                (8, "flight_state", "LOADED"),
                (9, "flight_log", "ACQUIRED"),
                (10, "close", "CLOSED_AND_ARCHIVE_READY"),
            ]
            for timestamp, parameter, value in evidence:
                session.record(
                    AutopilotTelemetrySourceAdapter(),
                    {
                        "timestamp_ms": timestamp,
                        "parameter": parameter,
                        "value": value,
                        "unit": "state",
                        "source": "ADAPTER_E2E",
                        "quality": "FIXTURE",
                        "context": {**common, "result": "PASS"},
                    },
                )
            session.finalize()
            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            events = [
                json.loads(line)
                for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines()
                if line.strip()
            ]
            self.assertEqual(record["flight_record_id"], "FLIGHT-RECORD-PH4-INT-027")
            self.assertEqual(record["mission_id"], "MISSION-027")
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(len(events), 10)
            for event in events:
                context = event["context"]
                self.assertEqual(context["vehicle_id"], "UAV-E2E-027")
                self.assertEqual(context["flight_record_id"], "FLIGHT-RECORD-PH4-INT-027")
                self.assertEqual(context["mission_id"], "MISSION-027")
                self.assertEqual(context["source_id"], "SRC-11")


if __name__ == "__main__":
    unittest.main()
