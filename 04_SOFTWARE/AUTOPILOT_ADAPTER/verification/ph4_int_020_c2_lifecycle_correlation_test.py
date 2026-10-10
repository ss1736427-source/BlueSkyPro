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


class Ph4Int020C2LifecycleCorrelationTest(unittest.TestCase):
    def test_c2_lifecycle_preserves_correlation(self) -> None:
        binary = ROOT / "build" / "autopilot-adapter" / "ph4_int_020_c2_lifecycle_correlation_fixture"
        self.assertTrue(binary.exists(), f"fixture binary missing: {binary}")

        line = subprocess.check_output([str(binary)], text=True).strip()
        parts = line.split("|")
        self.assertEqual(parts[0], "PH4_INT_020")
        flight_record_id, vehicle_id, protocol, protocol_version = parts[1:5]
        states = parts[5:]

        self.assertEqual(flight_record_id, "FLIGHT-RECORD-PH4-INT-020")
        self.assertEqual(vehicle_id, "UAV-C2-020")
        self.assertEqual(protocol, "BLUESKY-C2")
        self.assertEqual(protocol_version, "1.0")
        self.assertEqual(states[:8], [
            "PRECONNECT_REJECTED", "AUTH_REJECTED", "CONNECTED", "AUTHENTICATED",
            "HEARTBEAT_OK", "TX_RX_MATCH", "DISCONNECTED", "POSTDISCONNECT_REJECTED",
        ])
        self.assertRegex(states[8], r"^SEQ=[1-9][0-9]*$")

        common = {
            "protocol": protocol,
            "protocol_version": protocol_version,
            "vehicle_id": vehicle_id,
            "execution_source": "SIL_FIXTURE",
        }
        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(
                run_dir,
                "TEST-RUN-PH4-INT-020",
                evidence_domain_ids=["EC-20"],
                requirement_ids=["REQ-INT-C2-SIL-LIFECYCLE-CORRELATION"],
                test_method_id="TM-PH4-INT-020",
                test_case_id="TC-PH4-INT-020",
                configuration_id="CFG-SIL-C2-020",
                configuration={
                    "evidence_class": "SIL_FIXTURE_ONLY",
                    "protocol": protocol,
                    "protocol_version": protocol_version,
                },
                flight_record_id=flight_record_id,
                mission_id="MISSION-020",
            )
            events = [
                (1, "c2_state", "DISCONNECTED", "LINK_STATE"),
                (2, "c2_authentication", "REJECTED", "AUTHENTICATION"),
                (3, "c2_state", "CONNECTED", "LINK_STATE"),
                (4, "c2_authentication", "AUTHENTICATED", "AUTHENTICATION"),
                (5, "c2_heartbeat", "HEARTBEAT_OK", "HEARTBEAT"),
                (6, "c2_message", "TX_RX_MATCH", "MESSAGE_CORRELATION"),
                (7, "c2_state", "DISCONNECTED", "LINK_STATE"),
                (8, "c2_heartbeat", "REJECTED", "POSTDISCONNECT"),
            ]
            for timestamp, parameter, value, correlation in events:
                session.record(AutopilotTelemetrySourceAdapter(), {
                    "timestamp_ms": timestamp,
                    "parameter": parameter,
                    "value": value,
                    "unit": "state",
                    "source": "AUTOPILOT_TELEMETRY",
                    "quality": "FIXTURE",
                    "context": {**common, "correlation": correlation},
                })
            session.finalize()

            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            evidence = [json.loads(line) for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if line.strip()]
            self.assertEqual(record["flight_record_id"], flight_record_id)
            self.assertEqual(record["mission_id"], "MISSION-020")
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(len(evidence), 8)
            for event in evidence:
                self.assertEqual(event["context"]["flight_record_id"], flight_record_id)
                self.assertEqual(event["context"]["mission_id"], "MISSION-020")
                self.assertEqual(event["context"]["vehicle_id"], vehicle_id)
                self.assertEqual(event["context"]["protocol"], protocol)


if __name__ == "__main__":
    unittest.main()
