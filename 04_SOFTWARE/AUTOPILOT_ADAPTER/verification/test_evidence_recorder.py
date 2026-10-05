import json
import tempfile
import unittest
from pathlib import Path

from evidence_recorder import append_event, finalize


class EvidenceRecorderTest(unittest.TestCase):
    def test_collect_export_and_integrity_manifest(self):
        with tempfile.TemporaryDirectory() as temp:
            run_dir = Path(temp) / "COM-TEST-001"
            run_dir.mkdir()
            (run_dir / "record.json").write_text(
                json.dumps({
                    "test_run_id": "COM-TEST-001",
                    "status": "PENDING",
                    "requirement_ids": ["SYS-C2-006", "LOG-003"],
                    "test_method_id": "METHOD-COM-001",
                    "test_method_revision": "0.1",
                    "test_case_id": "CASE-COM-001",
                    "test_case_revision": "0.1",
                    "configuration": {"vehicle_id": "DEMO-UAV-01", "equipment": ["C2-DEMO"]},
                    "started_at_ms": 1000,
                    "ended_at_ms": 2000,
                    "events": []
                }, ensure_ascii=False), encoding="utf-8"
            )
            (run_dir / "events.jsonl").touch()
            append_event(run_dir, {"timestamp_ms": 1100, "parameter": "link_latency_ms", "value": 42.5, "unit": "ms", "source": "demo-c2"})
            append_event(run_dir, {"timestamp_ms": 1200, "parameter": "packet_loss", "value": 0.1, "unit": "%", "source": "demo-c2", "quality": "valid"})

            manifest, anchor, report = finalize(run_dir)
            data = json.loads(manifest.read_text(encoding="utf-8"))
            self.assertEqual(data["format"], "BlueSky-Evidence-1")
            self.assertEqual(len(data["files"]), 4)
            self.assertTrue((run_dir / "processed.csv").exists())
            self.assertTrue(anchor.exists())
            self.assertEqual(anchor.read_text(encoding="utf-8").split()[1], "manifest.json")
            self.assertTrue(report.exists())
            self.assertEqual(len((run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines()), 2)


if __name__ == "__main__":
    unittest.main()
