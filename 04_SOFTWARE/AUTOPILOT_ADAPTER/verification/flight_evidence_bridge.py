#!/usr/bin/env python3
"""Bridge operational flight facts into one BlueSky evidence session."""

from __future__ import annotations

from typing import Any, Mapping

from evidence_adapter import SourceAdapter
from evidence_session import EvidenceSession


class FlightEvidenceBridge:
    """Connect mission/flight identity and operational events to EvidenceSession."""

    def __init__(self, session: EvidenceSession) -> None:
        self.session = session
        self._validate_identity()

    def _validate_identity(self) -> None:
        record = self.session._load()
        required = ("session_id", "test_run_id", "configuration_id", "flight_record_id", "mission_id")
        missing = [key for key in required if not record.get(key)]
        if missing:
            raise ValueError(f"flight evidence session missing identity: {', '.join(missing)}")

    def record_flight_fact(
        self,
        adapter: SourceAdapter,
        *,
        timestamp_ms: int,
        parameter: str,
        value: Any,
        unit: str,
        source: str,
        quality: str | None = None,
        context: Mapping[str, Any] | None = None,
    ) -> None:
        """Record one operational fact while preserving session provenance."""
        payload = {
            "timestamp_ms": timestamp_ms,
            "parameter": parameter,
            "value": value,
            "unit": unit,
            "source": source,
            "quality": quality,
            "context": dict(context or {}),
        }
        self.session.record(adapter, payload)

    def record_flight_state(self, adapter: SourceAdapter, *, timestamp_ms: int, state: str) -> None:
        """Record a flight-state transition as a normal evidence event."""
        self.record_flight_fact(
            adapter,
            timestamp_ms=timestamp_ms,
            parameter="flight_state",
            value=state,
            unit="state",
            source="FLIGHT",
        )

    def record_telemetry(self, adapter: SourceAdapter, *, timestamp_ms: int, parameter: str, value: Any, unit: str, quality: str | None = None) -> None:
        """Record a telemetry parameter through the same evidence path."""
        self.record_flight_fact(
            adapter,
            timestamp_ms=timestamp_ms,
            parameter=parameter,
            value=value,
            unit=unit,
            source="TELEMETRY",
            quality=quality,
        )
