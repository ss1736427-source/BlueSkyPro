#!/usr/bin/env python3
"""Create a controlled adapter boundary for every registered evidence source.

Concrete hardware/protocol adapters can replace the generic boundary later
without changing the evidence storage contract.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Mapping

from evidence_adapter import SourceAdapter, EvidenceEvent
from evidence_source_registry import EvidenceSourceDefinition, get_source


@dataclass(frozen=True)
class RegisteredSourceAdapter(SourceAdapter):
    """Generic prototype adapter bound to one controlled registry entry."""

    definition: EvidenceSourceDefinition

    @property
    def source_type(self) -> str:  # type: ignore[override]
        return self.definition.source_type

    def normalize(self, record: Mapping[str, Any]) -> EvidenceEvent:
        event = super().normalize(record)
        context = dict(event.context)
        context.setdefault("source_id", self.definition.source_id)
        context.setdefault("evidence_domain_id", self.definition.domain_id)
        return EvidenceEvent(
            timestamp_ms=event.timestamp_ms,
            parameter=event.parameter,
            value=event.value,
            unit=event.unit,
            source=event.source,
            quality=event.quality,
            context=context,
        )

    def source_metadata(self) -> dict[str, str]:
        return {
            "source_id": self.definition.source_id,
            "source_type": self.definition.source_type,
            "evidence_domain_id": self.definition.domain_id,
        }


def adapter_for_source(source_type: str) -> RegisteredSourceAdapter:
    """Return the generic prototype adapter for a registered source."""
    return RegisteredSourceAdapter(get_source(source_type))
