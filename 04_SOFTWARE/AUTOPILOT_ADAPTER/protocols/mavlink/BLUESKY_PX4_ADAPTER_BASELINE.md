# BlueSky PRO — PX4 Adapter Baseline

**Status:** IMPLEMENTATION BASELINE — P0

## Purpose

Provide the second concrete implementation boundary between the universal BlueSky Autopilot API and a PX4 vehicle through the common MAVLink transport.

## Architecture

```text
BlueSky Universal API
        ↓
PX4 Adapter
        ↓
MAVLink Transport
        ↓
PX4
```

The PX4 adapter is a peer of the ArduPilot adapter. Neither adapter is allowed to introduce autopilot-specific semantics into BlueSky Core.

## Responsibilities

The adapter shall translate, validate and normalize PX4-specific semantics for:

- vehicle/system identification;
- capability discovery;
- vehicle/navigation state;
- telemetry;
- parameters/configuration;
- mission upload;
- mission read-back;
- command execution and result reporting;
- mode/arming state;
- failsafe and vehicle health state;
- connection/session status.

## Mission transfer

For a validated Mission Package the adapter shall:

1. verify target vehicle identity;
2. verify the active vehicle configuration;
3. compare required capabilities with PX4-reported capabilities where available;
4. translate the common mission representation into the PX4/MAVLink mission representation;
5. transfer the mission;
6. process protocol acknowledgements;
7. read back the executable mission where supported;
8. compare safety-critical executable content with the approved package;
9. return a deterministic result to BlueSky.

## Safety boundary

The adapter shall never bypass BlueSky release gates for safety, energy or regulatory constraints.

Upload failure, timeout, rejection or read-back mismatch shall produce a non-success result and shall prevent the package from being represented as safely loaded.

## Telemetry normalization

PX4 telemetry shall be mapped to the common BlueSky Unified Telemetry/Event Model while preserving:

- source system/component;
- source timestamp where available;
- receipt timestamp;
- data quality;
- source message identity;
- normalized interpretation.

Unsupported source data shall remain identifiable rather than being silently discarded when it can be operationally relevant.

## Parameter/configuration handling

PX4 parameter operations shall be isolated inside this adapter. Configuration changes affecting vehicle capability, safety, energy or mission execution shall invalidate the affected configuration/validation state according to the common lifecycle rules.

## Command handling

The adapter shall expose only commands represented by the Universal Autopilot API and supported by the active PX4 configuration.

Each command shall maintain the common lifecycle:

`REQUESTED → SENT → ACKNOWLEDGED → EXECUTED / REJECTED / TIMEOUT / UNKNOWN`

Protocol acknowledgement shall not by itself be interpreted as proof of physical execution where the command semantics require additional confirmation.

## Failsafe handling

PX4-specific failsafe states shall be normalized into the common runtime state model. The adapter reports vehicle state; the C2/runtime layer decides the BlueSky operational response according to the approved safety policy.

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

## Interoperability rule

The PX4 adapter must implement the same Universal Autopilot API contract as ArduPilot. Differences in native capability are represented through capability discovery and explicit unsupported/degraded results, not through changes to the BlueSky mission model.

## Acceptance criteria

The adapter is implementation-complete only when the Universal API can be exercised against a PX4 target, mission transfer can be verified, telemetry can be normalized, command outcomes can be correlated, and mandatory failure scenarios have objective verification evidence.

## Non-goals

This adapter does not define mission planning, algorithm orchestration, energy optimization, regulatory decision logic, C2 policy or UI behaviour.
