import tempfile
import unittest
from pathlib import Path

from evidence_ingestor import EvidenceIngestor


class TestEvidenceIngestor(unittest.TestCase):
    def test_registered_source_is_normalized_and_persisted(self):
        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp)
            ingestor = EvidenceIngestor(run_dir)
            event = ingestor.ingest(
                "C2_LINK",
                {
                    "timestamp_ms": 1234,
                    "parameter": "rssi",
                    "value": -72,
                    "unit": "dBm",
                    "source": "demo-c2",
                    "quality": "GOOD",
                },
            )
            self.assertEqual(event.context["source_id"], "SRC-01")
            self.assertEqual(event.context["evidence_domain_id"], "EC-01")
            lines = (run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines()
            self.assertEqual(len(lines), 1)
            self.assertIn('"parameter":"rssi"', lines[0])
            self.assertIn('"source_id":"SRC-01"', lines[0])

    def test_unknown_source_is_rejected_before_storage(self):
        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp)
            with self.assertRaises(KeyError):
                EvidenceIngestor(run_dir).ingest("NOT_REGISTERED", {})
            self.assertFalse((run_dir / "events.jsonl").exists())


if __name__ == "__main__":
    unittest.main()
