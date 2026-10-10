from __future__ import annotations

import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from certification_export import export_certification_index
from evidence_package_validator import validate_package


class TestCertificationExportValidation(unittest.TestCase):
    def _write_package(self, root: Path) -> None:
        record = {
            "test_run_id": "TR-EXPORT-001",
            "configuration_id": "CFG-EXPORT-001",
            "data_class": "DEVELOPMENT",
            "requirement_ids": ["REQ-001"],
            "test_method_id": "TM-001",
            "test_case_id": "TC-001",
            "sources": ["SRC-01"],
            "evidence_domain_ids": ["EC-01"],
        }
        (root / "record.json").write_text(json.dumps(record), encoding="utf-8")
        (root / "events.jsonl").write_text(
            json.dumps({"timestamp": 1, "parameter": "link", "value": 1, "unit": "state", "source": "SRC-01", "evidence_domain_id": "EC-01"}) + "\n",
            encoding="utf-8",
        )
        (root / "processed.csv").write_text("parameter,value\nlink,1\n", encoding="utf-8")
        (root / "report.md").write_text("# Result\n", encoding="utf-8")
        files = []
        for relative in ("record.json", "events.jsonl", "processed.csv", "report.md"):
            path = root / relative
            files.append({"path": relative, "sha256": hashlib.sha256(path.read_bytes()).hexdigest()})
        manifest = root / "manifest.json"
        manifest.write_text(json.dumps({"files": files}), encoding="utf-8")
        (root / "manifest.sha256").write_text(
            f"{hashlib.sha256(manifest.read_bytes()).hexdigest()}  manifest.json\n", encoding="utf-8"
        )

    def test_export_requires_valid_package(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self._write_package(root)
            output = export_certification_index(root)
            index = json.loads(output.read_text(encoding="utf-8"))
            self.assertEqual(index["test_run_id"], "TR-EXPORT-001")
            self.assertEqual(index["configuration_id"], "CFG-EXPORT-001")

    def test_export_rejects_tampered_package(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self._write_package(root)
            (root / "processed.csv").write_text("parameter,value\nlink,99\n", encoding="utf-8")
            errors = validate_package(root)
            self.assertIn("manifest.json:hash-mismatch:processed.csv", errors)
            with self.assertRaisesRegex(ValueError, "hash-mismatch:processed.csv"):
                export_certification_index(root)


if __name__ == "__main__":
    unittest.main()
