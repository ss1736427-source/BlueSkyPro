"""Generic evidence event capture for BlueSky PRO prototype and future tests.

The recorder is intentionally domain-agnostic: C2, navigation, flight control,
mission, equipment, power, weather, security, faults, and other evidence-bearing
sources use the same event envelope. It preserves source values and provenance
without declaring certification compliance.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
from typing import Any, Mapping


@dataclass(frozen=True)
class EvidenceEvent:
    event_id: str
    timestamp_utc: str
    evidence_domain_id: str
    source_id: str
    parameter: str
    value: Any
    unit: str | None
    quality: str | None
    test_run_id: str | None
    flight_record_id: str | None
    configuration_id: str | None
    evidence_class: str

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


class EvidenceCapture:
    """Append-only event capture with deterministic package metadata."""

    def __init__(self, root: str | Path):
        self.root = Path(root)
        self.raw = self.root / "raw"
        self.raw.mkdir(parents=True, exist_ok=True)
        self.events_file = self.root / "events.jsonl"

    @staticmethod
    def _utc_now() -> str:
        return datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")

    def record(
        self,
        *,
        event_id: str,
        evidence_domain_id: str,
        source_id: str,
        parameter: str,
        value: Any,
        unit: str | None = None,
        quality: str | None = None,
        test_run_id: str | None = None,
        flight_record_id: str | None = None,
        configuration_id: str | None = None,
        evidence_class: str = "DEMONSTRATION",
        timestamp_utc: str | None = None,
    ) -> EvidenceEvent:
        if not event_id or not evidence_domain_id or not source_id or not parameter:
            raise ValueError("event_id, evidence_domain_id, source_id and parameter are required")
        event = EvidenceEvent(
            event_id=event_id,
            timestamp_utc=timestamp_utc or self._utc_now(),
            evidence_domain_id=evidence_domain_id,
            source_id=source_id,
            parameter=parameter,
            value=value,
            unit=unit,
            quality=quality,
            test_run_id=test_run_id,
            flight_record_id=flight_record_id,
            configuration_id=configuration_id,
            evidence_class=evidence_class,
        )
        with self.events_file.open("a", encoding="utf-8") as handle:
            handle.write(json.dumps(event.to_dict(), ensure_ascii=False, separators=(",", ":")) + "\n")
        return event

    def export_raw_copy(self, source_name: str, data: bytes) -> Path:
        """Store an unchanged source payload and return its path."""
        if not source_name or Path(source_name).name != source_name:
            raise ValueError("source_name must be a simple file name")
        target = self.raw / source_name
        target.write_bytes(data)
        return target

    def manifest(self) -> dict[str, Any]:
        files: list[dict[str, Any]] = []
        for path in sorted(self.root.rglob("*")):
            if path.is_file() and path.name != "manifest.json":
                digest = hashlib.sha256(path.read_bytes()).hexdigest()
                files.append({"path": str(path.relative_to(self.root)), "sha256": digest, "size": path.stat().st_size})
        return {"format": "bluesky-evidence-package-v1", "generated_at_utc": self._utc_now(), "files": files}

    def write_manifest(self) -> Path:
        target = self.root / "manifest.json"
        target.write_text(json.dumps(self.manifest(), ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        return target
