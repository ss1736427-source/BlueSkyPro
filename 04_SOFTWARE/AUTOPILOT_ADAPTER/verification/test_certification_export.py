from __future__ import annotations

import json
from pathlib import Path

from certification_export import build_certification_index


def test_certification_index_preserves_traceability_and_integrity(tmp_path: Path) -> None:
    (tmp_path / "record.json").write_text(
        json.dumps({
            "test_run_id": "COM-TEST-001",
            "status": "FINALIZED",
            "data_class": "DEVELOPMENT",
            "requirement_ids": ["REQ-COM-001"],
            "test_method_id": "METHOD-COM-001",
            "test_method_revision": "A",
            "test_case_id": "TC-COM-001",
            "test_case_revision": "A",
            "configuration_id": "CFG-001",
            "flight_record_id": "FLT-001",
            "evidence_domain_ids": ["EC-01"],
            "sources": [{"source_type": "C2_LINK"}],
        }), encoding="utf-8"
    )
    (tmp_path / "events.jsonl").write_text("{}\n", encoding="utf-8")
    (tmp_path / "processed.csv").write_text("parameter,value\nrssi,-80\n", encoding="utf-8")
    (tmp_path / "evidence").mkdir()
    (tmp_path / "evidence" / "instrument.csv").write_text("value\n1\n", encoding="utf-8")

    index = build_certification_index(tmp_path)

    assert index["test_run_id"] == "COM-TEST-001"
    assert index["requirement_ids"] == ["REQ-COM-001"]
    assert index["test_method"]["revision"] == "A"
    assert index["configuration_id"] == "CFG-001"
    paths = {item["path"] for item in index["files"]}
    assert "record.json" in paths
    assert "evidence/instrument.csv" in paths
    assert all(len(item["sha256"]) == 64 for item in index["files"])
    assert index["result_boundary"]["certification_conclusion"] != "source data"
