# Multi-UAV Contract Validator

This directory contains the first executable contract-validation layer for BlueSky PRO's Multi-UAV planning pipeline.

## Scope

The harness validates the machine-readable contract and regression fixture semantics. It is **not** the flight planner, trajectory optimizer, collision-avoidance controller, regulatory authorization engine, or production Safety Engine.

## Commands

From repository root:

```text
python schemas/validator/multi_uav_contract_validator.py
python schemas/validator/test_multi_uav_regression_fixtures.py
```

If the optional `jsonschema` package is installed, the baseline validator also performs Draft 2020-12 schema validation. Without that package, the dependency-free semantic checks still run.

## Contract sequence

```text
Mission
  ↓
Constrained Open Space
  ↓
Zone Set
  ↓
Zone Assignment
  ↓
Route Set
  ↓
Performance Adjusted Route Set
  ↓
Trajectory Set
  ↓
Conflict Report
  ↓
Final Check
  ↓
RELEASE_ELIGIBLE / BLOCKED
```

## Test boundary

The current regression runner verifies the declared behavior of the fixture contracts, including:

- 4D conflict / unresolved classification;
- bounded 0–5 second temporal correction;
- rejection after secondary conflict;
- blocked state when no correction is permitted;
- downstream invalidation;
- deterministic replay identity;
- final-gate version mismatch;
- technical release vs authorization separation.

The next implementation layer can replace fixture-level assertions with executable domain objects and a real validator while preserving these contracts.


## Planning Core → HMI bridge

The Planning Core → HMI boundary uses `schemas/planning-core-bridge.schema.json`.

- Python serializes verified `PipelineResult` data through `planning_bridge_contract.py`.
- Qt `PlanningBridge::publishJson()` validates schema version, message type, mission/result identity and final verification status before exposing the result to QML.
- The bridge is transport-independent; no second planning engine is introduced.
- Transport/process integration is a separate runtime task and must consume this contract unchanged.

Validation command:

```text
python schemas/validator/test_planning_bridge_contract.py
```
