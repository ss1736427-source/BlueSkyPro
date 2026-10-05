import tempfile
import unittest
from pathlib import Path

from evidence_capture import EvidenceCapture


class EvidenceCaptureTests(unittest.TestCase):
    def test_common_event_envelope_and_manifest(self):
        with tempfile.TemporaryDirectory() as tmp:
            capture = EvidenceCapture(tmp)
            event = capture.record(
                event_id="EV-001",
                evidence_domain_id="EC-01",
                source_id="SRC-01",
                parameter="rssi",
                value=-82,
                unit="dBm",
                quality="VALID",
                test_run_id="TEST-001",
                configuration_id="CFG-001",
                evidence_class="DEMONSTRATION",
                timestamp_utc="2026-09-12T18:00:00Z",
            )
            self.assertEqual(event.evidence_domain_id, "EC-01")
            self.assertIn('"parameter":"rssi"', Path(tmp, "events.jsonl").read_text(encoding="utf-8"))
            raw = capture.export_raw_copy("source.bin", b"unchanged-source-data")
            self.assertEqual(raw.read_bytes(), b"unchanged-source-data")
            manifest = capture.write_manifest()
            text = manifest.read_text(encoding="utf-8")
            self.assertIn("events.jsonl", text)
            self.assertIn("source.bin", text)

    def test_required_identity_fields(self):
        with tempfile.TemporaryDirectory() as tmp:
            capture = EvidenceCapture(tmp)
            with self.assertRaises(ValueError):
                capture.record(event_id="", evidence_domain_id="EC-01", source_id="SRC-01", parameter="x", value=1)


if __name__ == "__main__":
    unittest.main()
