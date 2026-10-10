# PH4-INT-004 — Service Interface Integration Fixture

id: PH4-INT-004
status: IMPLEMENTATION_BASELINE

## Scope

Connect the PH4-INT execution fixture to the existing protocol-independent autopilot and C2 service interfaces without introducing protocol coupling.

## Delivered

- existing `UniversalAutopilotAdapter` boundary exercised through `AutopilotAdapterBaseline`;
- existing `UniversalC2Interface` boundary exercised through `C2InterfaceBaseline`;
- adapter connection and command acknowledgement;
- C2 authentication, heartbeat, dispatch and receive;
- mission write/read-back/semantic comparison;
- deterministic executable CMake/CTest fixture.

## Not claimed

- real C2 transport;
- real UAV connection;
- SIL/HIL verification;
- real-UAV verification;
- certification evidence.

## Gate

Complete after CI passes on the branch and the merged main baseline.

## Next deterministic gap

Add explicit failure-path propagation across the same service boundaries, including read-back mismatch blocking successful release and preservation of structured reason/error state.
