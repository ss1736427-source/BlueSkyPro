#!/usr/bin/env python3
"""Operational orchestration layer for multi-source evidence sessions."""

from __future__ import annotations

import json
import time
import uuid
from pathlib import Path
from typing import Any, Mapping

from evidence_adapter import SourceAdapter
from evidence_recorder import append_event, finalize
from evidence_package_validator import validate_package
from certification_export import export_certification_index


class EvidenceSession:
    """Collect and correlate facts from one or more registered sources."""

    STATES = ("INIT", "COLLECT", "FINALIZE", "VERIFY", "EXPORT", "ARCHIVE")

    def __init__(self, run_dir: Path, test_run_id: str, *, evidence_domain_ids: list[str] | None = None, requirement_ids: list[str] | None = None, test_method_id: str | None = None, test_method_revision: str | None = None, test_case_id: str | None = None, test_case_revision: str | None = None, configuration_id: str | None = None, configuration: Mapping[str, Any] | None = None, flight_record_id: str | None = None, mission_id: str | None = None, session_id: str | None = None, data_class: str = "DEMONSTRATION") -> None:
        if not test_run_id.strip():
            raise ValueError("test_run_id must not be empty")
        self.run_dir = Path(run_dir)
        self.run_dir.mkdir(parents=True, exist_ok=True)
        self.record_path = self.run_dir / "record.json"
        now = int(time.time() * 1000)
        self.session_id = session_id or f"SES-{uuid.uuid4().hex[:12].upper()}"
        record = {
            "session_id": self.session_id, "test_run_id": test_run_id, "status": "IN_PROGRESS", "lifecycle_state": "INIT", "data_class": data_class,
            "evidence_domain_ids": evidence_domain_ids or [], "requirement_ids": requirement_ids or [], "test_method_id": test_method_id, "test_method_revision": test_method_revision,
            "test_case_id": test_case_id, "test_case_revision": test_case_revision, "configuration_id": configuration_id, "flight_record_id": flight_record_id, "mission_id": mission_id,
            "configuration": dict(configuration or {}), "started_at_ms": now, "ended_at_ms": None, "events": [], "sources": [],
        }
        self.record_path.write_text(json.dumps(record, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        (self.run_dir / "events.jsonl").touch()

    def _load(self) -> dict[str, Any]:
        return json.loads(self.record_path.read_text(encoding="utf-8"))

    def _save(self, record: Mapping[str, Any]) -> None:
        self.record_path.write_text(json.dumps(record, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    def _transition(self, target: str) -> None:
        record = self._load()
        current = record["lifecycle_state"]
        allowed = {"INIT": {"COLLECT"}, "COLLECT": {"FINALIZE"}, "FINALIZE": {"VERIFY"}, "VERIFY": {"EXPORT"}, "EXPORT": {"ARCHIVE"}, "ARCHIVE": set()}
        if target not in allowed[current]:
            raise RuntimeError(f"invalid evidence session transition: {current} -> {target}")
        record["lifecycle_state"] = target
        self._save(record)

    def start_collection(self) -> None:
        self._transition("COLLECT")

    def record(self, adapter: SourceAdapter, source_record: Mapping[str, Any]) -> None:
        """Append one normalized source fact with session provenance."""
        record = self._load()
        if record["lifecycle_state"] == "INIT":
            self.start_collection()
            record = self._load()
        if record["lifecycle_state"] != "COLLECT":
            raise RuntimeError("evidence session is not accepting records")
        event_dict = adapter.normalize(source_record).as_dict()
        context = dict(event_dict.get("context", {}))
        context.update({"session_id": self.session_id, "test_run_id": record["test_run_id"], "configuration_id": record.get("configuration_id"), "flight_record_id": record.get("flight_record_id"), "mission_id": record.get("mission_id")})
        if "evidence_domain_id" not in context and len(record.get("evidence_domain_ids", [])) == 1:
            context["evidence_domain_id"] = record["evidence_domain_ids"][0]
        event_dict["context"] = context
        append_event(self.run_dir, event_dict)
        source = adapter.source_metadata()
        if source not in record["sources"]:
            record["sources"].append(source)
        self._save(record)

    def finalize(self):
        """Freeze collection, build, verify, export, and archive the package."""
        record = self._load()
        if record["lifecycle_state"] == "INIT":
            self.start_collection()
            record = self._load()
        self._transition("FINALIZE")
        record = self._load()
        record["ended_at_ms"] = int(time.time() * 1000)
        record["status"] = "ANALYZED"
        self._save(record)

        # Build the package for the FINALIZE state before entering VERIFY.
        result = finalize(self.run_dir)
        errors = validate_package(self.run_dir)
        if errors:
            raise ValueError("Evidence package verification failed: " + "; ".join(errors))

        # Lifecycle state is part of record.json and therefore part of its
        # manifest hash. Rebuild after each transition before verification.
        self._transition("VERIFY")
        result = finalize(self.run_dir)
        errors = validate_package(self.run_dir)
        if errors:
            raise ValueError("Evidence package verification failed: " + "; ".join(errors))

        export_certification_index(self.run_dir)
        self._transition("EXPORT")
        self._transition("ARCHIVE")

        # ARCHIVE is the final immutable lifecycle state. Rebuild the package
        # and certification index from that final self-consistent record.
        result = finalize(self.run_dir)
        export_certification_index(self.run_dir)
        errors = validate_package(self.run_dir)
        if errors:
            raise ValueError("Final evidence package verification failed: " + "; ".join(errors))
        return result
