# PH4-INT-017 — SIL Mission Execution / Readback Correlation

## Status
IMPLEMENTATION_BASELINE

## Objective
Extend the controlled SIL/MAVLink2 fixture from single command provenance to a complete mission execution/readback correlation sequence while preserving one Flight Record identity across all correlated events.

## Delivered
- Uploads one approved mission into the controlled SIL adapter fixture.
- Starts the mission and decodes the MAVLink2 command acknowledgement.
- Emits a correlated `EXECUTING` event bound to the same Flight Record, vehicle and command identity.
- Reads the mission back through the adapter and encodes/decodes the MAVLink2 mission-readback frame.
- Compares the approved mission with the decoded readback and requires an exact match.
- Emits a correlated `COMPLETED` event using the same Flight Record and command identity.
- Records execution, readback and comparison facts through the existing EvidenceSession boundary.
- Verifies that all evidence events retain Flight Record, mission, vehicle, command and protocol correlation.
- Adds deterministic CMake/CTest coverage.

## Correlation chain

```text
Approved Mission
      |
      v
Mission Upload
      |
      v
Start Mission
      |
      v
MAVLink2 CMD_ACK
      |
      v
SIL EXECUTING
      |
      v
Mission Readback
      |
      v
MAVLink2 MISSION_READBACK
      |
      v
Approved vs Readback Comparison
      |
      v
SIL COMPLETED
      |
      v
EvidenceSession / Flight Record
```

## Evidence class
SIL_FIXTURE_ONLY.

## Boundary
This is a controlled software fixture. It is not real MAVLink transport, HIL, real-aircraft execution, flight-test evidence, or certification evidence.

## Exit criterion
CI passes the CTest fixture and Python EvidenceSession correlation test with a single Flight Record identity preserved from mission execution through readback comparison and completion.

## Next deterministic gap
Add negative-path mission correlation coverage (readback mismatch / execution failure) before progressing toward broader SIL scenarios or HIL/physical execution.
