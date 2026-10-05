# PH4-INT-018 — Negative-Path Mission Correlation

## Status
IMPLEMENTATION_BASELINE

## Objective
Extend the controlled SIL/MAVLink2 mission execution/readback correlation chain with deterministic negative-path coverage while preserving the same Flight Record, mission, vehicle, command and protocol identity.

## Delivered
- Controlled SIL fixture with approved-vs-readback mission mismatch.
- Correlated `EXECUTING` event followed by `FAILED` execution state.
- Exact mission comparison required to report `MISMATCH`.
- Same Flight Record, mission, vehicle and command identities retained across negative-path evidence.
- EvidenceSession verification for mismatch and execution failure.
- CMake/CTest coverage and CI invocation.

## Evidence boundary
`SIL_FIXTURE_ONLY`.

No real MAVLink transport, HIL, real-aircraft execution, flight-test evidence, or certification evidence is claimed.

## Exit criterion
CI must pass the PH4-INT-018 negative-path fixture and EvidenceSession correlation test without error masking.

## Negative-path chain

```text
Approved Mission
      |
      v
Mission Upload (mismatched fixture mission)
      |
      v
Start Mission / MAVLink2 ACK
      |
      v
EXECUTING
      |
      +----> Mission Readback
      |             |
      |             v
      |       APPROVED != READBACK
      |             |
      |             v
      |         MISMATCH
      |
      v
FAILED
```

## Next step
Determine the next deterministic integration gap only after PH4-INT-018 reaches GREEN and is closed.
