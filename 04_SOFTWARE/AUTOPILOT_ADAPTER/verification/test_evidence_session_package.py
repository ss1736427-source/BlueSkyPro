import json
import tempfile
import unittest
from pathlib import Path

from certification_export import export_certification_index
from evidence_adapter import AutopilotTelemetrySourceAdapter
from evidence_package_validator import validate_package
from evidence_session import EvidenceSession


class TestEvidenceSessionPackage(unittest.TestCase):
    def test_session_finalize_verifies_and_exports(self):
        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            session = EvidenceSession(
                run_dir,
                "TR-SESSION-001",
                evidence_domain_ids=["EC-09"],
                requirement_ids=["REQ-09-001"],
                test_method_id="TM-09-001",
                test_case_id="TC-09-001",
                configuration_id="CFG-09-001",
                data_class="DEMONSTRATION",
            )
            session.record(AutopilotTelemetrySourceAdapter(), {
                "timestamp_ms": 1000,
                "parameter": "ground_speed",
                "value": 20.0,
                "unit": "m/s",
                "source": "SRC-09",
            })
            session.finalize()
            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            index = json.loads((run_dir / "certification_evidence_index.json").read_text(encoding="utf-8"))
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(record["status"], "ANALYZED")
            self.assertEqual(index["test_run_id"], "TR-SESSION-001")
            self.assertEqual(validate_package(run_dir), [])

    def test_export_rejects_invalid_package(self):
        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "invalid"
            run_dir.mkdir()
            (run_dir / "record.json").write_text(json.dumps({"test_run_id": "X"}), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "Invalid evidence package"):
                export_certification_index(run_dir)


if __name__ == "__main__":
    unittest.main()
