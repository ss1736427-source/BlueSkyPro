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


class Ph4Int021NavigationLifecycleCorrelationTest(unittest.TestCase):
    def test_navigation_lifecycle_preserves_source_and_vehicle_correlation(self) -> None:
        binary = ROOT / "build" / "autopilot-adapter" / "ph4_int_021_navigation_lifecycle_correlation_fixture"
        self.assertTrue(binary.exists(), f"fixture binary missing: {binary}")

        line = subprocess.check_output([str(binary)], text=True).strip()
        parts = line.split("|")
        self.assertEqual(parts[0], "PH4_INT_021")
        flight_record_id, vehicle_id = parts[1:3]
        protocol, protocol_version = parts[3:5]
        results = dict(item.split("=", 1) for item in parts[5:])

        self.assertEqual(flight_record_id, "FLIGHT-RECORD-PH4-INT-021")
        self.assertEqual(vehicle_id, "UAV-NAV-021")
        self.assertEqual(protocol, "GNSS_RTK_NTRIP")
        self.assertEqual(protocol_version, "BASELINE-1")
        self.assertEqual(results["SOURCE_REJECTED"], "PASS")
        self.assertEqual(results["VALID_ACCEPTED"], "PASS")
        self.assertEqual(results["READBACK"], "PASS")
        self.assertEqual(results["CORRELATION_MATCH"], "PASS")
        self.assertEqual(results["STALE_REJECTED"], "PASS")
        self.assertEqual(results["INVALID_REJECTED"], "PASS")
        self.assertEqual(results["QUALITY"], "USABLE")
        self.assertEqual(results["CORRECTION"], "HEALTHY")

        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(
                run_dir,
                "TEST-RUN-PH4-INT-021",
                evidence_domain_ids=["EC-03"],
                requirement_ids=["REQ-INT-NAVIGATION-LIFECYCLE-CORRELATION"],
                test_method_id="TM-PH4-INT-021",
                test_case_id="TC-PH4-INT-021",
                configuration_id="CFG-SIL-NAV-021",
                configuration={
                    "evidence_class": "SIL_FIXTURE_ONLY",
                    "protocol": protocol,
                    "protocol_version": protocol_version,
                },
                flight_record_id=flight_record_id,
                mission_id="MISSION-021",
            )
            common = {
                "protocol": protocol,
                "protocol_version": protocol_version,
                "vehicle_id": vehicle_id,
                "execution_source": "SIL_FIXTURE",
                "flight_record_id": flight_record_id,
                "mission_id": "MISSION-021",
                "source_id": "SRC-06",
            }
            evidence = [
                (1, "source_validation", "SOURCE_REJECTED", "SOURCE_ID_MISMATCH"),
                (2, "navigation_acceptance", "VALID_ACCEPTED", "VALID_SAMPLE"),
                (3, "navigation_readback", "READBACK", "READ_SAMPLE"),
                (4, "navigation_correlation", "CORRELATION_MATCH", "SOURCE_TO_VEHICLE"),
                (5, "navigation_quality", "USABLE", "QUALITY_STATE"),
                (6, "correction_state", "HEALTHY", "CORRECTION_STATE"),
                (7, "stale_data", "STALE_REJECTED", "STALE_SAMPLE"),
                (8, "invalid_data", "INVALID_REJECTED", "INVALID_SAMPLE"),
            ]
            for timestamp, parameter, value, correlation in evidence:
                session.record(
                    AutopilotTelemetrySourceAdapter(),
                    {
                        "timestamp_ms": timestamp,
                        "parameter": parameter,
                        "value": value,
                        "unit": "state",
                        "source": "NAVIGATION",
                        "quality": "FIXTURE",
                        "context": {**common, "correlation": correlation},
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
            self.assertEqual(record["mission_id"], "MISSION-021")
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(len(events), 8)
            for event in events:
                context = event["context"]
                self.assertEqual(context["flight_record_id"], flight_record_id)
                self.assertEqual(context["mission_id"], "MISSION-021")
                self.assertEqual(context["vehicle_id"], vehicle_id)
                self.assertEqual(context["protocol"], protocol)
                self.assertEqual(context["protocol_version"], protocol_version)
                self.assertEqual(context["source_id"], "SRC-06")


if __name__ == "__main__":
    unittest.main()
