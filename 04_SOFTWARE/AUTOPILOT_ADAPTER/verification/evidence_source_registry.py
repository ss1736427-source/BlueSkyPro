#!/usr/bin/env python3
"""Canonical registry of evidence-bearing source channels for BlueSky PRO.

The registry is the controlled bridge between evidence domains and concrete
source adapters. It does not assert physical availability; prototype sources
may be synthetic, replayed, bench, or simulated until physical verification.
"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class EvidenceSourceDefinition:
    source_id: str
    source_type: str
    domain_id: str
    description: str


EVIDENCE_SOURCES: tuple[EvidenceSourceDefinition, ...] = (
    EvidenceSourceDefinition("SRC-01", "C2_LINK", "EC-01", "Command and control channel"),
    EvidenceSourceDefinition("SRC-02", "AUTOPILOT_TELEMETRY", "EC-02", "Autopilot state and telemetry"),
    EvidenceSourceDefinition("SRC-03", "FLIGHT_CONTROL", "EC-04", "Flight-control execution facts"),
    EvidenceSourceDefinition("SRC-04", "MISSION", "EC-05", "Mission and route execution"),
    EvidenceSourceDefinition("SRC-05", "EQUIPMENT", "EC-06", "Installed equipment facts"),
    EvidenceSourceDefinition("SRC-06", "GNSS_RTK_NTRIP", "EC-03", "Navigation and reference data"),
    EvidenceSourceDefinition("SRC-07", "WEATHER", "EC-10", "Weather and environment"),
    EvidenceSourceDefinition("SRC-08", "ENERGY_POWER", "EC-07", "Energy, battery and power"),
    EvidenceSourceDefinition("SRC-09", "PROPULSION", "EC-08", "Propulsion system"),
    EvidenceSourceDefinition("SRC-10", "PERFORMANCE_MODEL", "EC-09", "Performance model calculations"),
    EvidenceSourceDefinition("SRC-11", "AIRSPACE_GEOFENCE", "EC-12", "Airspace and geofence data"),
    EvidenceSourceDefinition("SRC-12", "TIME_SYNCHRONIZATION", "EC-13", "Time and synchronization"),
    EvidenceSourceDefinition("SRC-13", "DATA_RECORDING_REPLAY", "EC-14", "Recording and replay"),
    EvidenceSourceDefinition("SRC-14", "BLUESKY_EVENT", "EC-15", "Internal BlueSky events"),
    EvidenceSourceDefinition("SRC-15", "OPERATOR_EVENT", "EC-15", "Factual operator events"),
    EvidenceSourceDefinition("SRC-16", "HMI", "EC-15", "Human-machine interface evidence"),
    EvidenceSourceDefinition("SRC-17", "EXTERNAL_INTEGRATION", "EC-16", "External system integration"),
    EvidenceSourceDefinition("SRC-18", "SECURITY_ACCESS", "EC-19", "Security and access control"),
    EvidenceSourceDefinition("SRC-19", "FAULT_RECOVERY", "EC-20", "Fault and recovery events"),
    EvidenceSourceDefinition("SRC-20", "MEASUREMENT_INSTRUMENT", "EC-20", "Independent measurements"),
    EvidenceSourceDefinition("SRC-21", "OTHER_COMMUNICATION", "EC-17", "Operational communication other than C2"),
)

SOURCE_TYPES = frozenset(source.source_type for source in EVIDENCE_SOURCES)
DOMAIN_IDS = frozenset(source.domain_id for source in EVIDENCE_SOURCES)


def get_source(source_type: str) -> EvidenceSourceDefinition:
    """Return the controlled definition for a source type."""
    for source in EVIDENCE_SOURCES:
        if source.source_type == source_type:
            return source
    raise KeyError(f"unknown evidence source type: {source_type}")


def all_source_types() -> tuple[str, ...]:
    """Return all controlled source types in registry order."""
    return tuple(source.source_type for source in EVIDENCE_SOURCES)


def sources_for_domain(domain_id: str) -> tuple[EvidenceSourceDefinition, ...]:
    """Return all registered sources contributing facts to a domain."""
    return tuple(source for source in EVIDENCE_SOURCES if source.domain_id == domain_id)
