# BlueSky PRO — ArduPilot Adapter Baseline

**Status:** IMPLEMENTATION BASELINE — P0

## Purpose

Provide the first concrete implementation boundary between the universal BlueSky Autopilot API and an ArduPilot vehicle through MAVLink.

## Responsibilities

The adapter shall translate, validate and normalize ArduPilot-specific semantics for:

- vehicle identification;
- capability discovery;
- vehicle state;
- telemetry;
- parameters/configuration;
- mission upload;
- mission read-back;
- command execution and result reporting;
- failsafe state;
- mode/arming state;
- connection/session status.

## Boundary

```text
BlueSky Universal API
        ↓
ArduPilot Adapter
        ↓
MAVLink Transport
        ↓
ArduPilot
```

ArduPilot-specific message IDs, modes, parameters and command mappings remain inside this adapter.

## Mission transfer

The adapter shall:

1. receive a validated Mission Package;
2. verify target vehicle/configuration identity;
3. verify supported mission capabilities;
4. translate package content into ArduPilot mission representation;
5. transfer it through MAVLink;
6. obtain protocol acknowledgements;
7. read back the mission where supported;
8. compare executable content with the approved package;
9. return a deterministic result to BlueSky.

## Safety rule

A failed upload, timeout, target rejection or read-back mismatch shall not be reported as a successfully loaded mission.

The adapter shall not bypass BlueSky safety, energy or regulatory release gates.

## Telemetry

ArduPilot telemetry shall be mapped into the common BlueSky telemetry/event model while preserving source identity, timestamps, quality and raw provenance where required.

## Parameters

Parameter operations shall be explicit and version-aware. A parameter change that affects vehicle capability, safety, energy or mission execution shall trigger the applicable configuration/revalidation workflow.

## Runtime commands

The adapter shall support only commands exposed by the Universal Autopilot API and confirmed by capability discovery/configuration. Unsupported commands return an explicit unsupported result.

## Initial verification matrix

| Function | SIL | HIL | Real UAV |
|---|---|---|---|
| Connect/heartbeat | required | required | required |
| Vehicle identification | required | required | required |
| Capability discovery | required | required | required |
| Telemetry normalization | required | required | required |
| Mission upload | required | required | required |
| Mission read-back | required | required | required |
| Command ACK/result | required | required | required |
| Parameter/configuration validation | required | required | required |
| C2 loss/recovery interaction | required | required | required |
| Failsafe state handling | required | required | required |

## Acceptance criteria

The adapter is implementation-complete only when the complete Universal API contract can be exercised against an ArduPilot target and all mandatory verification evidence is recorded.

## Non-goals

This adapter does not define:

- BlueSky mission planning;
- algorithm orchestration;
- energy optimization;
- regulatory decision logic;
- C2 policy;
- UI behaviour.

Those remain independent layers.
