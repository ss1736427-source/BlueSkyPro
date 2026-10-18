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
            "routeCandidate": {
                "routeId": "ROUTE-001", "routeVersion": "v1",
                "missionId": "MISSION-001", "missionVersion": "m1",
                "generatorId": "planner", "generatorVersion": "1",
                "calculationInputVersion": "input-1",
                "environmentSnapshotRefs": {
                    "terrainSnapshotId": "terrain-snapshot-001",
                    "airspaceSnapshotId": "airspace-snapshot-001",
                    "weatherSnapshotId": "weather-snapshot-001",
                    "windSnapshotId": "wind-snapshot-001",
                },
                "coordinateReferenceSystem": "EPSG:4326",
                "waypoints": [
                    {"waypointId": "WP-1", "latitudeDeg": 60.0, "longitudeDeg": 24.0,
                     "altitudeM": 100.0, "mandatory": False, "altitudeReference": "AGL"},
                    {"waypointId": "WP-2", "latitudeDeg": 60.1, "longitudeDeg": 24.1,
                     "altitudeM": 110.0, "mandatory": True, "altitudeReference": "AGL"},
                ],
            },
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
    assert message["result"]["routeCandidate"]["routeId"] == "ROUTE-001"
    assert message["result"]["routeCandidate"]["waypoints"][1]["mandatory"] is True
    assert message["result"]["routeCandidate"]["environmentSnapshotRefs"]["windSnapshotId"] == "wind-snapshot-001"


def test_stdio_adapter_does_not_promote_planar_routes() -> None:
    request = _request()
    request["inputs"].pop("routeCandidate")
    proc = subprocess.run(
        [sys.executable, str(ADAPTER)],
        input=json.dumps(request) + "\n",
        text=True,
        capture_output=True,
        check=True,
    )
    message = json.loads(proc.stdout.strip())
    assert "routeCandidate" not in message["result"]


def test_stdio_adapter_rejects_missing_environment_snapshot_refs() -> None:
    request = _request()
    request["inputs"]["routeCandidate"].pop("environmentSnapshotRefs")
    proc = subprocess.run(
        [sys.executable, str(ADAPTER)],
        input=json.dumps(request) + "\\n",
        text=True,
        capture_output=True,
        check=True,
    )
    assert not proc.stdout.strip()
    assert "ROUTE_CANDIDATE_ENVIRONMENT_SNAPSHOT_REFS_REQUIRED" in proc.stderr


def test_stdio_adapter_rejects_unknown_candidate_crs() -> None:
    request = _request()
    request["inputs"]["routeCandidate"]["coordinateReferenceSystem"] = "LOCAL_XY"
    proc = subprocess.run(
        [sys.executable, str(ADAPTER)],
        input=json.dumps(request) + "\n",
        text=True,
        capture_output=True,
        check=True,
    )
    assert not proc.stdout.strip()
    assert "ROUTE_CANDIDATE_CRS_UNSUPPORTED" in proc.stderr


if __name__ == "__main__":
    test_stdio_adapter_emits_planning_result()
    test_stdio_adapter_does_not_promote_planar_routes()
    test_stdio_adapter_rejects_missing_environment_snapshot_refs()
    test_stdio_adapter_rejects_unknown_candidate_crs()
    print("PLANNING CORE STDIO ADAPTER TESTS: PASS")
