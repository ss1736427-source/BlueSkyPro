"""Integration tests for the Planning Core JSONL stdio adapter."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent
ADAPTER = ROOT / "planning_core_stdio.py"


def _request() -> dict:
    points = [
        {"pointId": "R1:P001", "x": 0.0, "y": 0.0},
        {"pointId": "R1:P002", "x": 100.0, "y": 0.0},
    ]
    return {
        "schemaVersion": "1.0",
        "messageType": "planning.request",
        "missionId": "MISSION-001",
        "resultId": "RESULT-001",
        "source": {"plannerVersion": "test"},
        "traceability": {"inputRefs": ["fixture:001"]},
        "inputs": {
            "zoneStatus": "VERIFIED",
            "assignmentStatus": "VERIFIED",
            "routes": [{
                "routeId": "R1",
                "uavId": "UAV-1",
                "zoneId": "ZONE-1",
                "points": points,
                "length": 100.0,
                "verified": True,
            }],
            "performance": [{
                "routeId": "R1",
                "uavId": "UAV-1",
                "segments": [{
                    "segmentIndex": 0,
                    "distanceM": 100.0,
                    "groundSpeedMps": 10.0,
                    "energyWh": 10.0,
                    "windMarginMps": 5.0,
                }],
                "totalEnergyWh": 10.0,
                "reserveMarginWh": 90.0,
                "verified": True,
                "modelAuthority": "AUTHORITATIVE",
            }],
            "trajectories": [{
                "trajectoryId": "T1",
                "routeId": "R1",
                "uavId": "UAV-1",
                "startTimeS": 0.0,
                "endTimeS": 10.0,
                "points": [{
                    "pointId": "R1:P001",
                    "x": 0.0,
                    "y": 0.0,
                    "altitudeM": 100.0,
                    "timestampS": 0.0,
                    "groundSpeedMps": 10.0,
                }, {
                    "pointId": "R1:P002",
                    "x": 100.0,
                    "y": 0.0,
                    "altitudeM": 100.0,
                    "timestampS": 10.0,
                    "groundSpeedMps": 10.0,
                }],
                "verified": True,
            }],
            "minimums": {"horizontalM": 20.0, "verticalM": 10.0},
            "resolutionPolicy": {"maxDelayS": 5.0},
        },
    }


def test_stdio_adapter_emits_planning_result() -> None:
    proc = subprocess.run(
        [sys.executable, str(ADAPTER)],
        input=json.dumps(_request()) + "\n",
        text=True,
        capture_output=True,
        check=True,
    )
    assert not proc.stderr
    message = json.loads(proc.stdout.strip())
    assert message["messageType"] == "planning.result"
    assert message["missionId"] == "MISSION-001"
    assert message["verification"]["finalGateStatus"] == "PASS"
    assert message["verification"]["releaseStatus"] == "RELEASE_ELIGIBLE"


if __name__ == "__main__":
    test_stdio_adapter_emits_planning_result()
    print("PLANNING CORE STDIO ADAPTER TESTS: PASS")
