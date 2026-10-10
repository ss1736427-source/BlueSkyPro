from __future__ import annotations

import hashlib
import json
from pathlib import Path

from evidence_package_validator import validate_package


def _write_package(root: Path, *, canonical_events: bool = False) -> None:
    record = {"test_run_id":"TR-001","configuration_id":"CFG-001","data_class":"DEVELOPMENT","requirement_ids":["REQ-001"],"test_method_id":"TM-001","test_case_id":"TC-001","sources":["SRC-01"],"evidence_domain_ids":["EC-01"]}
    (root / "record.json").write_text(json.dumps(record), encoding="utf-8")
    if canonical_events:
        event = {"event_id":"EV-001","timestamp_utc":"2026-09-14T20:00:00Z","evidence_domain_id":"EC-01","source_id":"SRC-01","parameter":"link","value":1,"unit":"state","quality":"VALID","test_run_id":"TR-001","flight_record_id":"FR-001","configuration_id":"CFG-001","evidence_class":"DEVELOPMENT"}
    else:
        event = {"timestamp":1,"parameter":"link","value":1,"unit":"state","source":"SRC-01","evidence_domain_id":"EC-01"}
    (root / "events.jsonl").write_text(json.dumps(event)+"\n", encoding="utf-8")
    (root / "processed.csv").write_text("parameter,value\nlink,1\n", encoding="utf-8")
    (root / "report.md").write_text("# Result\n", encoding="utf-8")
    (root / "evidence").mkdir()
    (root / "evidence" / "source.log").write_text("raw\n", encoding="utf-8")
    files=[]
    for relative in ("record.json","events.jsonl","processed.csv","report.md","evidence/source.log"):
        path=root/relative
        files.append({"path":relative,"sha256":hashlib.sha256(path.read_bytes()).hexdigest()})
    (root/"manifest.json").write_text(json.dumps({"files":files}), encoding="utf-8")
    manifest_sha = hashlib.sha256((root/"manifest.json").read_bytes()).hexdigest()
    (root/"manifest.sha256").write_text(manifest_sha + "  manifest.json\n", encoding="utf-8")


def test_valid_package(tmp_path: Path) -> None:
    _write_package(tmp_path)
    assert validate_package(tmp_path) == []


def test_canonical_source_event_is_accepted(tmp_path: Path) -> None:
    _write_package(tmp_path, canonical_events=True)
    assert validate_package(tmp_path) == []


def test_hash_mismatch_is_rejected(tmp_path: Path) -> None:
    _write_package(tmp_path)
    (tmp_path / "processed.csv").write_text("parameter,value\nlink,2\n", encoding="utf-8")
    assert "manifest.json:hash-mismatch:processed.csv" in validate_package(tmp_path)


def test_manifest_sha_mismatch_is_rejected(tmp_path: Path) -> None:
    _write_package(tmp_path)
    (tmp_path / "manifest.sha256").write_text("0" * 64 + "  manifest.json\n", encoding="utf-8")
    assert "manifest.sha256:hash-mismatch:manifest.json" in validate_package(tmp_path)


def test_missing_traceability_is_rejected(tmp_path: Path) -> None:
    _write_package(tmp_path)
    record = json.loads((tmp_path / "record.json").read_text(encoding="utf-8"))
    del record["configuration_id"]
    (tmp_path / "record.json").write_text(json.dumps(record), encoding="utf-8")
    assert "record:configuration_id" in validate_package(tmp_path)
