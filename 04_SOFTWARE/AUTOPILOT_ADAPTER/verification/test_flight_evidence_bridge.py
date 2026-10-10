#!/usr/bin/env python3
"""Tests for the operational flight-to-evidence bridge."""

from __future__ import annotations

import json
from pathlib import Path
import tempfile

from evidence_adapter import AutopilotTelemetrySourceAdapter
from evidence_session import EvidenceSession
from flight_evidence_bridge import FlightEvidenceBridge


def test_flight_fact_keeps_operational_identity() -> None:
    with tempfile.TemporaryDirectory() as tmp:
        run_dir = Path(tmp)
        session = EvidenceSession(
            run_dir,
            "TR-FLIGHT-001",
            evidence_domain_ids=["EC-04", "EC-14"],
            configuration_id="CFG-001",
            flight_record_id="FLT-001",
            mission_id="MIS-001",
        )
        bridge = FlightEvidenceBridge(session)
        bridge.record_telemetry(
            AutopilotTelemetrySourceAdapter(),
            timestamp_ms=1000,
            parameter="battery_voltage",
            value=24.6,
            unit="V",
            quality="VALID",
        )

        event = json.loads((run_dir / "events.jsonl").read_text(encoding="utf-8").splitlines()[0])
        context = event["context"]
        assert context["session_id"] == session.session_id
        assert context["test_run_id"] == "TR-FLIGHT-001"
        assert context["configuration_id"] == "CFG-001"
        assert context["flight_record_id"] == "FLT-001"
        assert context["mission_id"] == "MIS-001"
        assert event["source"] == "TELEMETRY"
        assert event["value"] == 24.6


def test_bridge_rejects_session_without_flight_identity() -> None:
    with tempfile.TemporaryDirectory() as tmp:
        session = EvidenceSession(Path(tmp), "TR-FLIGHT-002", configuration_id="CFG-001")
        try:
            FlightEvidenceBridge(session)
        except ValueError as exc:
            assert "flight_record_id" in str(exc)
        else:
            raise AssertionError("bridge accepted a session without flight identity")


if __name__ == "__main__":
    test_flight_fact_keeps_operational_identity()
    test_bridge_rejects_session_without_flight_identity()
    print("flight evidence bridge tests: PASS")
