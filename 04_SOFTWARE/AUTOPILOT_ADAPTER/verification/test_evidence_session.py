import json
import tempfile
import unittest
from pathlib import Path

from evidence_adapter import C2LinkSourceAdapter
from evidence_session import EvidenceSession


class TestEvidenceSession(unittest.TestCase):
    def _session(self, run_dir: Path, test_run_id: str = "RUN-001") -> EvidenceSession:
        return EvidenceSession(
            run_dir,
            test_run_id,
            evidence_domain_ids=["EC-14"],
            requirement_ids=["REQ-001"],
            test_method_id="METHOD-001",
            test_method_revision="A",
            test_case_id="CASE-001",
            test_case_revision="A",
            configuration_id="CFG-001",
            configuration={"configuration_id": "CFG-001"},
            flight_record_id="FLIGHT-001",
            mission_id="MISSION-001",
            data_class="DEMONSTRATION",
        )

    def test_source_to_package_flow_and_provenance(self):
        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "COM-TEST-001"
            session = EvidenceSession(
                run_dir,
                "COM-TEST-001",
                evidence_domain_ids=["EC-01", "EC-14"],
                requirement_ids=["SYS-C2-006"],
                test_method_id="METHOD-COM-001",
                test_method_revision="A",
                test_case_id="CASE-COM-001",
                test_case_revision="A",
                configuration_id="CFG-DEMO-001",
                configuration={"configuration_id": "CFG-DEMO-001"},
                flight_record_id="FLIGHT-DEMO-001",
                mission_id="MISSION-DEMO-001",
                data_class="DEMONSTRATION",
            )
            session.record(
                C2LinkSourceAdapter(),
                {
                    "timestamp_ms": 1000,
                    "parameter": "link_latency_ms",
                    "value": 42.5,
                    "unit": "ms",
                    "source": "demo-c2",
                    "context": {"evidence_domain_id": "EC-14"},
                },
            )
            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            self.assertEqual(record["lifecycle_state"], "COLLECT")
            self.assertEqual(record["sources"], [{"source_type": "C2_LINK"}])

            event = json.loads((run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines()[0])
            self.assertEqual(event["context"]["session_id"], session.session_id)
            self.assertEqual(event["context"]["test_run_id"], "COM-TEST-001")
            self.assertEqual(event["context"]["evidence_domain_id"], "EC-14")

            manifest, anchor, report = session.finalize()
            record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
            self.assertEqual(record["lifecycle_state"], "ARCHIVE")
            self.assertEqual(record["status"], "ANALYZED")
            self.assertEqual(len(record["events"]), 1)
            self.assertTrue(manifest.exists())
            self.assertTrue(anchor.exists())
            self.assertTrue(report.exists())
            self.assertTrue((run_dir / "certification_evidence_index.json").exists())

    def test_recording_after_archive_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            session = self._session(Path(tmp) / "RUN")
            session.record(
                C2LinkSourceAdapter(),
                {"timestamp_ms": 1, "parameter": "x", "value": 1, "unit": "u", "source": "demo"},
            )
            session.finalize()
            with self.assertRaises(RuntimeError):
                session.record(
                    C2LinkSourceAdapter(),
                    {"timestamp_ms": 2, "parameter": "x", "value": 2, "unit": "u", "source": "demo"},
                )


if __name__ == "__main__":
    unittest.main()
