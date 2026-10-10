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

class Ph4Int025CleanCloseTest(unittest.TestCase):
    def test_clean_close_lifecycle_and_archive_correlation(self) -> None:
        binary = ROOT / "build" / "autopilot-adapter" / "ph4_int_025_clean_close_fixture"
        self.assertTrue(binary.exists(), f"fixture binary missing: {binary}")
        parts = subprocess.check_output([str(binary)], text=True).strip().split("|")
        self.assertEqual(parts[:5], ["PH4_INT_025", "FLIGHT-RECORD-PH4-INT-025", "UAV-CLOSE-025", "MAVLINK2", "BASELINE-1"])
        results = dict(item.split("=", 1) for item in parts[5:])
        for key in ("CAPABILITY", "CLOSED", "DISCONNECTED", "IDEMPOTENT", "EXECUTION_BLOCKED", "RECONNECT_BOUNDARY", "FINAL_CLOSED", "STATE_BOUNDARY"):
            self.assertEqual(results[key], "PASS")

        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(run_dir, "TEST-RUN-PH4-INT-025", evidence_domain_ids=["EC-03"], requirement_ids=["REQ-INT-CLEAN-CLOSE"], test_method_id="TM-PH4-INT-025", test_case_id="TC-PH4-INT-025", configuration_id="CFG-SIL-CLOSE-025", configuration={"evidence_class": "SIL_FIXTURE_ONLY", "protocol": "MAVLINK2", "protocol_version": "BASELINE-1"}, flight_record_id="FLIGHT-RECORD-PH4-INT-025", mission_id="MISSION-025")
            common = {"protocol": "MAVLINK2", "protocol_version": "BASELINE-1", "vehicle_id": "UAV-CLOSE-025", "execution_source": "SIL_FIXTURE", "flight_record_id": "FLIGHT-RECORD-PH4-INT-025", "mission_id": "MISSION-025", "source_id": "SRC-10"}
            evidence = [(1, "close", "CLOSED_AND_ARCHIVE_READY", "PASS"), (2, "connection", "DISCONNECTED", "PASS"), (3, "repeat_close", "ALREADY_CLOSED", "PASS"), (4, "post_close_execution", "BLOCKED", "PASS"), (5, "reconnect_boundary", "RESTORED", "PASS"), (6, "final_close", "CLOSED_AND_ARCHIVE_READY", "PASS")]
            for timestamp, parameter, value, result in evidence:
                session.record(AutopilotTelemetrySourceAdapter(), {"timestamp_ms": timestamp, "parameter": parameter, "value": value, "unit": "state", "source": "ADAPTER_CLOSE", "quality": "FIXTURE", "context": {**common, "result": result}})
            session.finalize()
            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            events = [json.loads(line) for line in (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines() if line.strip()]
            self.assertEqual(record["flight_record_id"], "FLIGHT-RECORD-PH4-INT-025")
            self.assertEqual(record["mission_id"], "MISSION-025")
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(len(events), 6)
            for event in events:
                context = event["context"]
                self.assertEqual(context["vehicle_id"], "UAV-CLOSE-025")
                self.assertEqual(context["flight_record_id"], "FLIGHT-RECORD-PH4-INT-025")
                self.assertEqual(context["mission_id"], "MISSION-025")
                self.assertEqual(context["source_id"], "SRC-10")

if __name__ == "__main__":
    unittest.main()
