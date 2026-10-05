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


class Ph4Int018NegativePathMissionCorrelationTest(unittest.TestCase):
    def test_negative_paths_preserve_correlation(self) -> None:
        binary = ROOT / "build" / "autopilot-adapter" / "ph4_int_018_negative_path_mission_correlation_fixture"
        self.assertTrue(binary.exists(), f"fixture binary missing: {binary}")

        line = subprocess.check_output([str(binary)], text=True).strip()
        parts = line.split("|", 8)
        self.assertEqual(parts[0], "PH4_INT_018")
        flight_record_id, vehicle_id, command_id = parts[1:4]
        executing_state, failed_state = parts[4:6]
        protocol, protocol_version = parts[6:8]
        framed_payload = parts[8]

        marker = "|MAVLINK2|MISSION_READBACK|"
        marker_index = framed_payload.find(marker)
        self.assertGreater(marker_index, 0)
        start_frame = framed_payload[:marker_index]
        tail = framed_payload[marker_index + len(marker):]

        suffix = "|MISMATCH|EXECUTION_FAILURE"
        self.assertTrue(tail.endswith(suffix))
        tail = tail[:-len(suffix)]
        mission_fields = tail.split("|")
        self.assertEqual(len(mission_fields), 3)
        actual_mission, approved_mission, decoded_mission = mission_fields
        self.assertEqual(actual_mission, decoded_mission)
        readback_frame = "MAVLINK2|MISSION_READBACK|" + actual_mission

        self.assertEqual(flight_record_id, "FLIGHT-RECORD-PH4-INT-018")
        self.assertEqual(vehicle_id, "UAV-MAV-018")
        self.assertTrue(command_id)
        self.assertEqual(executing_state, "EXECUTING")
        self.assertEqual(failed_state, "FAILED")
        self.assertEqual(protocol, "MAVLink2")
        self.assertEqual(protocol_version, "2")
        self.assertTrue(start_frame.startswith("MAVLINK2|CMD_ACK|"))
        self.assertTrue(readback_frame.startswith("MAVLINK2|MISSION_READBACK|"))
        self.assertEqual(approved_mission, "MISSION-018:UAV-MAV-018")
        self.assertEqual(actual_mission, "MISSION-018:UAV-MAV-018-MISMATCH")
        self.assertNotEqual(actual_mission, approved_mission)

        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(
                run_dir,
                "TEST-RUN-PH4-INT-018",
                evidence_domain_ids=["EC-18"],
                requirement_ids=["REQ-INT-MAVLINK2-SIL-NEGATIVE-CORRELATION"],
                test_method_id="TM-PH4-INT-018",
                test_case_id="TC-PH4-INT-018",
                configuration_id="CFG-SIL-MAVLINK2-018",
                configuration={
                    "evidence_class": "SIL_FIXTURE_ONLY",
                    "protocol": protocol,
                    "protocol_version": protocol_version,
                },
                flight_record_id=flight_record_id,
                mission_id="MISSION-018",
            )
            common = {
                "protocol": protocol,
                "protocol_version": protocol_version,
                "vehicle_id": vehicle_id,
                "command_id": command_id,
                "execution_source": "SIL_FIXTURE",
            }
            session.record(AutopilotTelemetrySourceAdapter(), {
                "timestamp_ms": 1, "parameter": "mission_execution_state",
                "value": executing_state, "unit": "state", "source": "AUTOPILOT_TELEMETRY",
                "quality": "FIXTURE", "context": {**common, "correlation": "EXECUTION"},
            })
            session.record(AutopilotTelemetrySourceAdapter(), {
                "timestamp_ms": 2, "parameter": "mission_readback",
                "value": actual_mission, "unit": "mission", "source": "AUTOPILOT_TELEMETRY",
                "quality": "FIXTURE", "context": {**common, "protocol_frame": readback_frame, "correlation": "READBACK"},
            })
            session.record(AutopilotTelemetrySourceAdapter(), {
                "timestamp_ms": 3, "parameter": "mission_comparison",
                "value": "MISMATCH", "unit": "result", "source": "AUTOPILOT_TELEMETRY",
                "quality": "FIXTURE", "context": {**common, "correlation": "APPROVED_VS_READBACK", "approved_mission": approved_mission},
            })
            session.record(AutopilotTelemetrySourceAdapter(), {
                "timestamp_ms": 4, "parameter": "mission_execution_state",
                "value": failed_state, "unit": "state", "source": "AUTOPILOT_TELEMETRY",
                "quality": "FIXTURE", "context": {**common, "correlation": "EXECUTION", "failure": "EXECUTION_FAILURE"},
            })
            session.finalize()

            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            events = [json.loads(line) for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if line.strip()]
            self.assertEqual(record["flight_record_id"], flight_record_id)
            self.assertEqual(record["mission_id"], "MISSION-018")
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(len(events), 4)
            self.assertTrue(any(e["value"] == "MISMATCH" for e in events))
            self.assertTrue(any(e["value"] == "FAILED" for e in events))
            for event in events:
                self.assertEqual(event["context"]["flight_record_id"], flight_record_id)
                self.assertEqual(event["context"]["mission_id"], "MISSION-018")
                self.assertEqual(event["context"]["vehicle_id"], vehicle_id)
                self.assertEqual(event["context"]["command_id"], command_id)
                self.assertEqual(event["context"]["protocol"], protocol)


if __name__ == "__main__":
    unittest.main()
