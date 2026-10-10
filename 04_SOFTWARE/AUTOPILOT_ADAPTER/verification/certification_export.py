#!/usr/bin/env python3
"""Build a deterministic certification-evidence index from one evidence run.

The exporter never changes source records. It creates a portable index that
references the original record, processed data, supporting evidence and their
integrity hashes. Acceptance is kept separate from measured facts.
"""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from evidence_package_validator import validate_package


PACKAGE_FILES = (
    "record.json",
    "events.jsonl",
    "processed.csv",
    "manifest.json",
    "manifest.sha256",
    "report.md",
)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def build_certification_index(run_dir: Path) -> dict[str, Any]:
    """Create a certification-facing index only from a valid evidence package."""
    run_dir = Path(run_dir)
    errors = validate_package(run_dir)
    if errors:
        raise ValueError("Invalid evidence package: " + "; ".join(errors))

    record = json.loads((run_dir / "record.json").read_text(encoding="utf-8"))
    files: list[dict[str, Any]] = []
    for relative in PACKAGE_FILES:
        path = run_dir / relative
        files.append({
            "path": relative,
            "sha256": sha256_file(path),
            "size_bytes": path.stat().st_size,
        })

    evidence_dir = run_dir / "evidence"
    if evidence_dir.exists():
        for path in sorted(p for p in evidence_dir.rglob("*") if p.is_file()):
            files.append({
                "path": str(path.relative_to(run_dir)).replace("\\", "/"),
                "sha256": sha256_file(path),
                "size_bytes": path.stat().st_size,
            })

    return {
        "format": "BLUESKY_CERTIFICATION_EVIDENCE_INDEX_V1",
        "test_run_id": record["test_run_id"],
        "data_class": record.get("data_class"),
        "requirement_ids": record.get("requirement_ids", []),
        "test_method": {
            "id": record.get("test_method_id"),
            "revision": record.get("test_method_revision"),
        },
        "test_case": {
            "id": record.get("test_case_id"),
            "revision": record.get("test_case_revision"),
        },
        "configuration_id": record.get("configuration_id"),
        "flight_record_id": record.get("flight_record_id"),
        "evidence_domain_ids": record.get("evidence_domain_ids", []),
        "sources": record.get("sources", []),
        "status": record.get("status"),
        "files": files,
        "result_boundary": {
            "measured_values": "source data",
            "calculated_values": "engineering analysis",
            "acceptance": "verification procedure",
            "certification_conclusion": "competent verification/certification process",
        },
    }


def export_certification_index(run_dir: Path, output: Path | None = None) -> Path:
    run_dir = Path(run_dir)
    output = output or (run_dir / "certification_evidence_index.json")
    index = build_certification_index(run_dir)
    output.write_text(json.dumps(index, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return output


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Export a BlueSky PRO certification evidence index")
    parser.add_argument("run_dir", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    print(export_certification_index(args.run_dir, args.output))
