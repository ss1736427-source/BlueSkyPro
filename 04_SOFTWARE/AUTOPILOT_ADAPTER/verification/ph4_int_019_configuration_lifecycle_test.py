#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
VERIFICATION = ROOT / "04_SOFTWARE" / "AUTOPILOT_ADAPTER" / "verification"
sys.path.insert(0, str(VERIFICATION))

from evidence_adapter import AutopilotTelemetrySourceAdapter
from evidence_session import EvidenceSession


class Ph4Int019ConfigurationLifecycleTest(unittest.TestCase):
    def test_configuration_lifecycle_preserves_verification_boundary(self) -> None:
        binary = ROOT / "build" / "autopilot-adapter" / "ph4_int_019_configuration_lifecycle_fixture"
        self.assertTrue(binary.exists(), f"fixture binary missing: {binary}")

        line = subprocess.check_output([str(binary)], text=True).strip()
        parts = line.split("|")
        self.assertEqual(parts[0], "PH4_INT_019")
        flight_record_id, vehicle_id = parts[1:3]
        protocol, protocol_version = parts[3:5]
        results = parts[5:]

        self.assertEqual(flight_record_id, "FLIGHT-RECORD-PH4-INT-019")
        self.assertEqual(vehicle_id, "UAV-MAV-019")
        self.assertEqual(protocol, "MAVLink2")
        self.assertEqual(protocol_version, "2")
        self.assertEqual(
            results,
            ["BASELINE_EMPTY", "VALIDATION_REJECTED", "WRITE", "READBACK", "MATCH", "VERIFIED", "MISMATCH", "VERIFY_FAILED"],
        )

        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(
                run_dir,
                "TEST-RUN-PH4-INT-019",
                evidence_domain_ids=["EC-19"],
                requirement_ids=["REQ-INT-CONFIGURATION-LIFECYCLE"],
                test_method_id="TM-PH4-INT-019",
                test_case_id="TC-PH4-INT-019",
                configuration_id="CFG-SIL-MAVLINK2-019",
                configuration={
                    "evidence_class": "SIL_FIXTURE_ONLY",
                    "protocol": protocol,
                    "protocol_version": protocol_version,
                },
                flight_record_id=flight_record_id,
                mission_id="CONFIG-019",
            )
            common = {
                "protocol": protocol,
                "protocol_version": protocol_version,
                "vehicle_id": vehicle_id,
                "execution_source": "SIL_FIXTURE",
            }
            evidence = [
                (1, "configuration_baseline", "BASELINE_EMPTY", "READ_BASELINE"),
                (2, "configuration_validation", "VALIDATION_REJECTED", "VALIDATE"),
                (3, "configuration_write", "WRITE", "WRITE"),
                (4, "configuration_readback", "READBACK", "READ_BACK"),
                (5, "configuration_comparison", "MATCH", "COMPARE"),
                (6, "configuration_verification", "VERIFIED", "VERIFY"),
                (7, "configuration_comparison", "MISMATCH", "COMPARE_NEGATIVE"),
                (8, "configuration_verification", "VERIFY_FAILED", "VERIFY_NEGATIVE"),
            ]
            for timestamp, parameter, value, correlation in evidence:
                session.record(
                    AutopilotTelemetrySourceAdapter(),
                    {
                        "timestamp_ms": timestamp,
                        "parameter": parameter,
                        "value": value,
                        "unit": "state",
                        "source": "AUTOPILOT_TELEMETRY",
                        "quality": "FIXTURE",
                        "context": {**common, "flight_record_id": flight_record_id, "mission_id": "CONFIG-019", "correlation": correlation},
                    },
                )
            session.finalize()

            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            events = [
                json.loads(line)
                for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines()
                if line.strip()
            ]
            self.assertEqual(record["flight_record_id"], flight_record_id)
            self.assertEqual(record["mission_id"], "CONFIG-019")
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(len(events), 8)
            self.assertTrue(any(e["value"] == "VERIFIED" for e in events))
            self.assertTrue(any(e["value"] == "VERIFY_FAILED" for e in events))
            for event in events:
                self.assertEqual(event["context"]["flight_record_id"], flight_record_id)
                self.assertEqual(event["context"]["mission_id"], "CONFIG-019")
                self.assertEqual(event["context"]["vehicle_id"], vehicle_id)
                self.assertEqual(event["context"]["protocol"], protocol)
                self.assertEqual(event["context"]["protocol_version"], protocol_version)


if __name__ == "__main__":
    unittest.main()
