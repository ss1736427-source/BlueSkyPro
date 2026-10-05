# BlueSky PRO — Operational Decision Evidence Contract 001

**Status:** IMPLEMENTATION BASELINE — P0
**Parent:** `04_SOFTWARE/OPERATIONS/BLUESKY_OPERATIONAL_ORCHESTRATOR_INTEGRATION_CONTRACT_001.md`

## Purpose
Record consequential orchestration decisions so the selected action, authority, constraints and source state remain auditable and replayable.

## Minimum evidence
Each recorded selection carries:
- stable decision ID;
- orchestrator policy version;
- input operational-state version;
- considered action IDs;
- selected action ID, when one exists;
- applicable constraints;
- resolved authority;
- resolved automation level;
- outcome (`Selected`, `HumanRequired`, or `None`);
- resulting state transition;
- completion evidence when available.

## Boundary
The recorder records decisions; it does not execute actions, alter authority, redefine subsystem state or bypass Safety, Regulatory or Energy gates. It is independent of UI and FCS execution.

A no-selection result remains a valid evidence record with `None` outcome so blocked states are not silently lost.

## Verification
CTest covers a selected automatic action and a no-selection case, including identity, policy/state provenance, actions, constraints, authority, automation, outcome and transition/completion evidence.

## Anti-rework rule
This contract establishes evidence continuity before resource/mission adaptation and Runtime integration. Later components may extend provenance without changing the meaning of these fields.
