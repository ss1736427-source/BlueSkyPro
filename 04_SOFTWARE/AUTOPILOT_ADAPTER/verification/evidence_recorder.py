#!/usr/bin/env python3
"""Portable prototype for collecting and exporting BlueSky verification evidence."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
from pathlib import Path
from typing import Any


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def append_event(run_dir: Path, event: dict[str, Any]) -> None:
    required = ("timestamp_ms", "parameter", "value", "unit", "source")
    missing = [name for name in required if name not in event]
    if missing:
        raise ValueError(f"missing event fields: {', '.join(missing)}")
    events = run_dir / "events.jsonl"
    with events.open("a", encoding="utf-8") as stream:
        stream.write(json.dumps(event, ensure_ascii=False, separators=(",", ":")) + "\n")


def load_events(run_dir: Path) -> list[dict[str, Any]]:
    events = run_dir / "events.jsonl"
    if not events.exists():
        return []
    result: list[dict[str, Any]] = []
    for line in events.read_text(encoding="utf-8").splitlines():
        if line.strip():
            result.append(json.loads(line))
    return result


def export_csv(run_dir: Path, events: list[dict[str, Any]]) -> Path:
    output = run_dir / "processed.csv"
    fields = ["timestamp_ms", "parameter", "value", "unit", "source", "quality", "context"]
    with output.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for event in events:
            row = {key: event.get(key, "") for key in fields}
            row["context"] = json.dumps(event.get("context", {}), ensure_ascii=False, separators=(",", ":"))
            writer.writerow(row)
    return output


def finalize(run_dir: Path) -> tuple[Path, Path, Path]:
    record_path = run_dir / "record.json"
    events = load_events(run_dir)
    record = json.loads(record_path.read_text(encoding="utf-8"))
    record["events"] = events
    record_path.write_text(json.dumps(record, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    csv_path = export_csv(run_dir, events)
    report_path = run_dir / "report.md"
    report = [
        f"# Verification Record {record['test_run_id']}",
        "",
        f"**Status:** {record['status']}",
        f"**Requirements:** {', '.join(record.get('requirement_ids', [])) or 'not assigned'}",
        f"**Test method:** {record.get('test_method_id') or 'not assigned'}",
        f"**Events collected:** {len(events)}",
        "",
        "## Configuration",
        "",
        "```json",
        json.dumps(record.get("configuration", {}), ensure_ascii=False, indent=2),
        "```",
        "",
        "## Export",
        "",
        "- Raw event stream: `events.jsonl`",
        f"- Tabular export: `{csv_path.name}`",
        "- Integrity manifest: `manifest.json`",
        "- Manifest integrity anchor: `manifest.sha256`",
        "- Supporting originals: `evidence/`",
        "",
        "> This report is a generated evidence index. Regulatory acceptance is determined by the approved test procedure and competent verification/certification process.",
        "",
    ]
    report_path.write_text("\n".join(report), encoding="utf-8")

    controlled = [record_path, run_dir / "events.jsonl", csv_path, report_path]
    manifest_entries = []
    for path in controlled:
        if path.exists():
            manifest_entries.append({"path": path.relative_to(run_dir).as_posix(), "size": path.stat().st_size, "sha256": sha256(path)})
    manifest = {"format": "BlueSky-Evidence-1", "test_run_id": record["test_run_id"], "files": manifest_entries}
    manifest_path = run_dir / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    anchor_path = run_dir / "manifest.sha256"
    anchor_path.write_text(f"{sha256(manifest_path)}  manifest.json\n", encoding="utf-8")
    return manifest_path, anchor_path, report_path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)

    init = sub.add_parser("init")
    init.add_argument("run_dir", type=Path)
    init.add_argument("test_run_id")
    init.add_argument("--status", default="PENDING", choices=["PENDING", "NOT_PERFORMED", "IN_PROGRESS", "ANALYZED", "PASS", "FAIL", "INVALID"])

    event = sub.add_parser("event")
    event.add_argument("run_dir", type=Path)
    event.add_argument("json_event")

    final = sub.add_parser("finalize")
    final.add_argument("run_dir", type=Path)

    args = parser.parse_args()
    if args.command == "init":
        args.run_dir.mkdir(parents=True, exist_ok=True)
        record = {"test_run_id": args.test_run_id, "status": args.status, "requirement_ids": [], "test_method_id": None, "test_method_revision": None, "test_case_id": None, "test_case_revision": None, "configuration": {}, "started_at_ms": None, "ended_at_ms": None, "events": []}
        (args.run_dir / "record.json").write_text(json.dumps(record, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        (args.run_dir / "events.jsonl").touch()
        return 0
    if args.command == "event":
        append_event(args.run_dir, json.loads(args.json_event))
        return 0
    manifest, anchor, report = finalize(args.run_dir)
    print(f"manifest={manifest}")
    print(f"manifest_sha256={anchor}")
    print(f"report={report}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
