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


class Ph4Int017MissionExecutionReadbackCorrelationTest(unittest.TestCase):
    def test_complete_sil_mission_execution_readback_correlation(self) -> None:
        binary = ROOT / "build" / "autopilot-adapter" / "ph4_int_017_mission_execution_readback_correlation_fixture"
        self.assertTrue(binary.exists(), f"fixture binary missing: {binary}")

        line = subprocess.check_output([str(binary)], text=True).strip()
        parts = line.split("|", 8)
        self.assertEqual(parts[0], "PH4_INT_017")
        flight_record_id, vehicle_id, command_id = parts[1:4]
        executing_state, completed_state = parts[4:6]
        protocol, protocol_version = parts[6:8]
        framed_payload = parts[8]

        readback_marker = "|MAVLINK2|MISSION_READBACK|"
        self.assertIn(readback_marker, framed_payload)
        start_frame, readback_and_results = framed_payload.split(readback_marker, 1)
        start_frame = start_frame.rstrip("|")
        readback_payload, approved_mission, actual_mission, comparison = readback_and_results.rsplit("|", 3)
        readback_frame = "MAVLINK2|MISSION_READBACK|" + readback_payload

        self.assertEqual(flight_record_id, "FLIGHT-RECORD-PH4-INT-017")
        self.assertEqual(vehicle_id, "UAV-MAV-017")
        self.assertTrue(command_id)
        self.assertEqual(executing_state, "EXECUTING")
        self.assertEqual(completed_state, "COMPLETED")
        self.assertEqual(protocol, "MAVLink2")
        self.assertEqual(protocol_version, "2")
        self.assertTrue(start_frame.startswith("MAVLINK2|CMD_ACK|"))
        self.assertTrue(readback_frame.startswith("MAVLINK2|MISSION_READBACK|"))
        self.assertEqual(approved_mission, "MISSION-017:UAV-MAV-017")
        self.assertEqual(actual_mission, approved_mission)
        self.assertEqual(comparison, "MATCH")

        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(
                run_dir,
                "TEST-RUN-PH4-INT-017",
                evidence_domain_ids=["EC-17"],
                requirement_ids=["REQ-INT-MAVLINK2-SIL-MISSION-CORRELATION"],
                test_method_id="TM-PH4-INT-017",
                test_case_id="TC-PH4-INT-017",
                configuration_id="CFG-SIL-MAVLINK2-017",
                configuration={
                    "evidence_class": "SIL_FIXTURE_ONLY",
                    "protocol": protocol,
                    "protocol_version": protocol_version,
                },
                flight_record_id=flight_record_id,
                mission_id="MISSION-017",
            )

            common = {
                "protocol": protocol,
                "protocol_version": protocol_version,
                "vehicle_id": vehicle_id,
                "command_id": command_id,
                "execution_source": "SIL_FIXTURE",
            }
            session.record(AutopilotTelemetrySourceAdapter(), {
                "timestamp_ms": 1,
                "parameter": "mission_execution_state",
                "value": executing_state,
                "unit": "state",
                "source": "AUTOPILOT_TELEMETRY",
                "quality": "FIXTURE",
                "context": {**common, "protocol_frame": start_frame, "correlation": "EXECUTION"},
            })
            session.record(AutopilotTelemetrySourceAdapter(), {
                "timestamp_ms": 2,
                "parameter": "mission_readback",
                "value": actual_mission,
                "unit": "mission",
                "source": "AUTOPILOT_TELEMETRY",
                "quality": "FIXTURE",
                "context": {**common, "protocol_frame": readback_frame, "correlation": "READBACK"},
            })
            session.record(AutopilotTelemetrySourceAdapter(), {
                "timestamp_ms": 3,
                "parameter": "mission_comparison",
                "value": comparison,
                "unit": "result",
                "source": "AUTOPILOT_TELEMETRY",
                "quality": "FIXTURE",
                "context": {**common, "correlation": "APPROVED_VS_READBACK", "approved_mission": approved_mission},
            })
            session.record(AutopilotTelemetrySourceAdapter(), {
                "timestamp_ms": 4,
                "parameter": "mission_execution_state",
                "value": completed_state,
                "unit": "state",
                "source": "AUTOPILOT_TELEMETRY",
                "quality": "FIXTURE",
                "context": {**common, "correlation": "EXECUTION"},
            })
            session.finalize()

            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            events = [json.loads(line) for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if line.strip()]
            self.assertEqual(record["flight_record_id"], flight_record_id)
            self.assertEqual(record["mission_id"], "MISSION-017")
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(len(events), 4)
            for event in events:
                self.assertEqual(event["context"]["flight_record_id"], flight_record_id)
                self.assertEqual(event["context"]["mission_id"], "MISSION-017")
                self.assertEqual(event["context"]["vehicle_id"], vehicle_id)
                self.assertEqual(event["context"]["command_id"], command_id)
                self.assertEqual(event["context"]["protocol"], protocol)


if __name__ == "__main__":
    unittest.main()
