---
id: SYS-REQ-086-CAPABILITY-TRACEABILITY
type: requirement_traceability
requirement: SYS-REQ-086
---

# SYS-REQ-086 — Graceful Degradation Traceability

## Architecture Traceability

| SYS-REQ-086 | ARCH-DEC-017 | - | PRIMARY | Error Handling and Recovery explicitly defines the DEGRADED condition and controlled responses including continuation, adaptation, resource reallocation, UAV role change, mission-scope reduction, replanning and abort. |
| SYS-REQ-086 | ARCH-DEC-039 | - | SUPPORTING | Runtime Adaptation classifies degradation-related runtime changes and determines the required validation and authority for controlled adaptation or replanning. |
| SYS-REQ-086 | ARCH-DEC-023 | - | SUPPORTING | Runtime and Recovery modes provide controlled operational-mode transitions, validation and recovery/adaptation paths when runtime conditions change. |

## Degradation Control

The architecture supports controlled degradation through:

DETECT
→ CLASSIFY AS DEGRADED
→ ASSESS MISSION FEASIBILITY
→ CONTINUE / ADAPT / REALLOCATE / REDUCE SCOPE / REPLAN / ABORT
→ REVALIDATE
→ SAFETY GATE

Safety constraints remain authoritative during degradation handling.

## Priority Preservation

SYS-REQ-086 requires preservation of P0 and P1 functions and retention of P2 functionality required for safe mission execution.

The referenced architecture decisions establish the safety authority and degraded-operation mechanisms but do not independently define the complete P0-P5 degradation policy.

## Coverage Assessment

SYS-REQ-086 is architecturally covered by ARCH-DEC-017 as the primary degraded-condition and recovery mechanism, with ARCH-DEC-039 and ARCH-DEC-023 providing runtime adaptation, validation and operational-mode support.

The exact P0-P5 degradation policy and workload classification remain subject to lower-level design and verification.


