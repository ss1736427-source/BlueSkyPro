# BlueSky Universal Integration Execution Contract 001

id: BLUESKY-UNIVERSAL-INTEGRATION-EXECUTION-CONTRACT-001
status: IMPLEMENTATION_BASELINE
work_package: PH4-INT-002

## Purpose

Define the deterministic service boundary that links the existing Universal Autopilot API, compatibility resolution, mission compiler, adapter, C2, read-back, and evidence services without binding the BlueSky core to a protocol implementation.

## Canonical execution chain

`Canonical Mission Package → Compatibility Resolution → Mission Compilation → Adapter Dispatch → C2 Transport → Vehicle/Aircraft → State & Mission Read-back → Semantic Comparison → Evidence Record`

A stage may reject execution. A rejection is a controlled result and must not be converted into a silent fallback.

## Service contracts

| Stage | Input | Output | Blocking condition |
|---|---|---|---|
| Compatibility | canonical vehicle + mission requirements | compatibility result | incompatible version, missing capability |
| Compiler | canonical mission + resolved capabilities | adapter mission package | unsupported feature / invalid mission |
| Adapter | adapter mission package | dispatch result | protocol/adapter error |
| C2 | command/mission transport request | acknowledgement + transport result | link loss / timeout / authentication failure |
| Read-back | vehicle state + mission state | normalized state/mission | unavailable or inconsistent read-back |
| Comparison | planned canonical mission + read-back | semantic comparison | safety-critical mismatch |
| Evidence | normalized lifecycle events + source records | evidence references | missing provenance/integrity |

## Required lifecycle

1. `PREPARE` — load canonical mission and vehicle configuration.
2. `RESOLVE` — resolve compatibility and capabilities.
3. `COMPILE` — produce adapter-specific mission package.
4. `VALIDATE` — validate mission, authority, safety and configuration gates.
5. `CONNECT` — establish and verify C2 link.
6. `DISPATCH` — transfer command/mission through the adapter.
7. `ACKNOWLEDGE` — record acknowledgement or controlled rejection.
8. `READ_BACK` — obtain normalized vehicle/mission state where supported.
9. `COMPARE` — perform semantic comparison against the canonical release package.
10. `RELEASE` — permit execution only when mandatory gates pass.
11. `EVIDENCE` — persist the lifecycle result with provenance.

## Invariants

- Canonical BlueSky objects do not contain ArduPilot, PX4, MAVLink or OEM protocol types.
- Compatibility is resolved before mission compilation.
- Capability validation precedes upload/dispatch.
- Safety-critical mismatch blocks release.
- C2 transport does not define mission semantics.
- Adapter protocol details remain inside the adapter boundary.
- Read-back is normalized before comparison.
- Evidence records reference source and configuration identity.
- Real UAV execution is not implied by this contract; SIL/HIL/real-UAV verification remain separate maturity gates.

## Minimum deterministic result

Every execution attempt produces:

- `execution_id`
- `vehicle_id`
- `mission_id`
- `adapter_id`
- `lifecycle_state`
- `result`
- `reason_code`
- `source_timestamp`
- `adapter_timestamp`
- `configuration_id`
- `evidence_reference`

## Error propagation

Allowed controlled errors are the errors defined by `universal_autopilot_api.yaml`. An adapter must preserve the semantic error class when translating protocol-specific failures.

## Verification status

This contract establishes the integration boundary only. It does **not** grant SIL, HIL, real-UAV, certification, or product-readiness status.
