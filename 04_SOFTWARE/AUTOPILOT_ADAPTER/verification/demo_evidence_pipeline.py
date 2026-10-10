#!/usr/bin/env python3
"""Run a small end-to-end demonstration of the evidence pipeline."""

from __future__ import annotations

import argparse
from pathlib import Path

from evidence_adapter import AutopilotTelemetrySourceAdapter, C2LinkSourceAdapter
from evidence_session import EvidenceSession


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run_dir", type=Path)
    args = parser.parse_args()

    session = EvidenceSession(
        args.run_dir,
        "DEMO-TEST-001",
        requirement_ids=["SYS-C2-006"],
        test_method_id="METHOD-COM-001",
        test_method_revision="DEMO",
        test_case_id="CASE-COM-001",
        test_case_revision="DEMO",
        configuration={"configuration_id": "CFG-DEMO-001", "vehicle": "DEMO-UAV"},
        data_class="DEMONSTRATION",
    )
    session.record(
        C2LinkSourceAdapter(),
        {"timestamp_ms": 1000, "parameter": "link_latency_ms", "value": 42.5, "unit": "ms", "source": "demo-c2"},
    )
    session.record(
        C2LinkSourceAdapter(),
        {"timestamp_ms": 1100, "parameter": "packet_loss", "value": 0.1, "unit": "%", "source": "demo-c2"},
    )
    session.record(
        AutopilotTelemetrySourceAdapter(),
        {"timestamp_ms": 1200, "parameter": "flight_mode", "value": "AUTO", "unit": "enum", "source": "demo-autopilot"},
    )
    manifest, anchor, report = session.finalize()
    print(f"evidence_package={args.run_dir}")
    print(f"manifest={manifest}")
    print(f"manifest_sha256={anchor}")
    print(f"report={report}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
