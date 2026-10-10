#!/usr/bin/env python3
"""Common source-to-evidence boundary for the BlueSky PRO prototype.

The adapter normalizes source records without changing the source payload. It is
intentionally independent of the concrete autopilot, C2 channel, equipment, or
measurement instrument so those real sources can replace demo sources later.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, Mapping


@dataclass(frozen=True)
class EvidenceEvent:
    """Canonical event passed to the evidence recorder."""

    timestamp_ms: int
    parameter: str
    value: Any
    unit: str
    source: str
    quality: str | None = None
    context: Mapping[str, Any] = field(default_factory=dict)

    def as_dict(self) -> dict[str, Any]:
        event: dict[str, Any] = {
            "timestamp_ms": self.timestamp_ms,
            "parameter": self.parameter,
            "value": self.value,
            "unit": self.unit,
            "source": self.source,
        }
        if self.quality is not None:
            event["quality"] = self.quality
        if self.context:
            event["context"] = dict(self.context)
        return event


class SourceAdapter:
    """Base boundary for converting a source record to an evidence event."""

    source_type = "UNSPECIFIED"

    def normalize(self, record: Mapping[str, Any]) -> EvidenceEvent:
        required = ("timestamp_ms", "parameter", "value", "unit", "source")
        missing = [key for key in required if key not in record]
        if missing:
            raise ValueError(f"missing source fields: {', '.join(missing)}")
        return EvidenceEvent(
            timestamp_ms=int(record["timestamp_ms"]),
            parameter=str(record["parameter"]),
            value=record["value"],
            unit=str(record["unit"]),
            source=str(record["source"]),
            quality=None if record.get("quality") is None else str(record["quality"]),
            context=dict(record.get("context", {})),
        )

    def source_metadata(self) -> dict[str, str]:
        return {"source_type": self.source_type}


class C2LinkSourceAdapter(SourceAdapter):
    """Adapter for command-and-control channel facts."""

    source_type = "C2_LINK"


class AutopilotTelemetrySourceAdapter(SourceAdapter):
    """Adapter for autopilot telemetry facts."""

    source_type = "AUTOPILOT_TELEMETRY"


class EquipmentSourceAdapter(SourceAdapter):
    """Adapter for installed equipment facts."""

    source_type = "EQUIPMENT"


class GnssRtkNtripSourceAdapter(SourceAdapter):
    """Adapter for GNSS/RTK/NTRIP navigation and reference facts."""

    source_type = "GNSS_RTK_NTRIP"


class WeatherSourceAdapter(SourceAdapter):
    """Adapter for weather and environmental observations."""

    source_type = "WEATHER"


class MeasurementInstrumentSourceAdapter(SourceAdapter):
    """Adapter for independent measurement-instrument facts."""

    source_type = "MEASUREMENT_INSTRUMENT"


class BlueSkyEventSourceAdapter(SourceAdapter):
    """Adapter for factual internal BlueSky events."""

    source_type = "BLUESKY_EVENT"


class OperatorEventSourceAdapter(SourceAdapter):
    """Adapter for factual operator observations/events."""

    source_type = "OPERATOR_EVENT"
