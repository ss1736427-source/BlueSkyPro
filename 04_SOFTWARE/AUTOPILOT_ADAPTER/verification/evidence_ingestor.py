#!/usr/bin/env python3
"""Route normalized evidence events from registered sources into a test run."""

from __future__ import annotations

from pathlib import Path
from typing import Any, Mapping

from evidence_adapter import EvidenceEvent
from evidence_recorder import append_event
from evidence_source_adapter_factory import adapter_for_source


class EvidenceIngestor:
    """Prototype ingestion boundary shared by all evidence-bearing channels."""

    def __init__(self, run_dir: Path) -> None:
        self.run_dir = run_dir

    def ingest(self, source_type: str, record: Mapping[str, Any]) -> EvidenceEvent:
        """Normalize one source record and append the resulting evidence event."""
        adapter = adapter_for_source(source_type)
        event = adapter.normalize(record)
        payload = {
            "timestamp_ms": event.timestamp_ms,
            "parameter": event.parameter,
            "value": event.value,
            "unit": event.unit,
            "source": event.source,
            "quality": event.quality,
            "context": dict(event.context),
        }
        self.run_dir.mkdir(parents=True, exist_ok=True)
        append_event(self.run_dir, payload)
        return event
