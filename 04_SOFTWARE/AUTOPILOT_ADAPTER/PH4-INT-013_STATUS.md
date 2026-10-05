# PH4-INT-013 — Heterogeneous Adapter Conformance

## Status
IMPLEMENTATION_BASELINE

## Objective
Prove that heterogeneous UAV identity/protocol profiles can use the same universal autopilot contract without changing the upper-layer mission execution and evidence interfaces.

## Delivered
- Fixed-wing and multirotor representative profiles use the same `UniversalAutopilotAdapter` contract.
- Vehicle type, autopilot identity, firmware and protocol are preserved independently per adapter instance.
- Common connect, mission-state and execution-command semantics are exercised for both profiles.
- Each profile is bound to its own Flight Record and EvidenceSession.
- CI executes the heterogeneous conformance fixture.

## Evidence class
DEMONSTRATION / fixture-only.

## Verification boundary
This establishes contract-level heterogeneity using baseline adapter implementations. It does not constitute verification of real aircraft, a real autopilot, SIL/HIL, or certification.

## Next deterministic gap
Introduce a protocol-specific representative adapter behind the universal contract and connect it to a controlled SIL/HIL execution source.
