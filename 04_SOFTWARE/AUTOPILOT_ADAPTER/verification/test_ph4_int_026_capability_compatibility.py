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


class Ph4Int026CapabilityCompatibilityTest(unittest.TestCase):
    def test_manifest_compatibility_and_provenance(self) -> None:
        binary = ROOT / "build" / "autopilot-adapter" / "ph4_int_026_capability_compatibility_fixture"
        self.assertTrue(binary.exists(), f"fixture binary missing: {binary}")
        parts = subprocess.check_output([str(binary)], text=True).strip().split("|")
        self.assertEqual(parts[:5], ["PH4_INT_026", "FLIGHT-RECORD-PH4-INT-026", "UAV-COMPAT-026", "MAVLINK2", "BASELINE-1"])
        results = dict(item.split("=", 1) for item in parts[5:])
        for key in ("MANIFEST", "COMPATIBLE", "PROTOCOL_MISMATCH", "VERSION_MISMATCH", "CAPABILITY_MISSING", "IDENTITY", "CAPABILITY_SETS", "VERIFICATION_STATUS"):
            self.assertEqual(results[key], "PASS")

        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(
                run_dir,
                "TEST-RUN-PH4-INT-026",
                evidence_domain_ids=["EC-03"],
                requirement_ids=["REQ-INT-CAPABILITY-COMPATIBILITY"],
                test_method_id="TM-PH4-INT-026",
                test_case_id="TC-PH4-INT-026",
                configuration_id="CFG-SIL-COMPAT-026",
                configuration={"evidence_class": "SIL_FIXTURE_ONLY", "protocol": "MAVLINK2", "protocol_version": "BASELINE-1"},
                flight_record_id="FLIGHT-RECORD-PH4-INT-026",
                mission_id="MISSION-026",
            )
            common = {
                "protocol": "MAVLINK2",
                "protocol_version": "BASELINE-1",
                "vehicle_id": "UAV-COMPAT-026",
                "execution_source": "SIL_FIXTURE",
                "flight_record_id": "FLIGHT-RECORD-PH4-INT-026",
                "mission_id": "MISSION-026",
                "source_id": "SRC-11",
            }
            evidence = [
                (1, "manifest", "COMPLETE", "PASS"),
                (2, "compatible", "ACCEPTED", "PASS"),
                (3, "protocol_mismatch", "REJECTED", "PASS"),
                (4, "version_mismatch", "REJECTED", "PASS"),
                (5, "capability_missing", "REJECTED", "PASS"),
            ]
            for timestamp, parameter, value, result in evidence:
                session.record(AutopilotTelemetrySourceAdapter(), {
                    "timestamp_ms": timestamp,
                    "parameter": parameter,
                    "value": value,
                    "unit": "state",
                    "source": "ADAPTER_COMPATIBILITY",
                    "quality": "FIXTURE",
                    "context": {**common, "result": result},
                })
            session.finalize()
            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            events = [json.loads(line) for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if line.strip()]
            self.assertEqual(record["flight_record_id"], "FLIGHT-RECORD-PH4-INT-026")
            self.assertEqual(record["mission_id"], "MISSION-026")
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(len(events), 5)
            for event in events:
                context = event["context"]
                self.assertEqual(context["vehicle_id"], "UAV-COMPAT-026")
                self.assertEqual(context["flight_record_id"], "FLIGHT-RECORD-PH4-INT-026")
                self.assertEqual(context["mission_id"], "MISSION-026")
                self.assertEqual(context["source_id"], "SRC-11")


if __name__ == "__main__":
    unittest.main()
