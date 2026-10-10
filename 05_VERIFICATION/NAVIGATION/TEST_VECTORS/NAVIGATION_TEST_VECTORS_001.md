# BLUE SKY PRO — NAVIGATION TEST VECTORS

Status: CONTROLLED WORKING BASELINE
Purpose: deterministic test vectors for the mathematical Navigation specification.

## 1. Scope

This document defines verification vectors for the Navigation mathematical layer. It does not introduce new system requirements and does not assign certification thresholds that are not established by the controlled project basis.

## 2. Vector set

| ID | Function | Inputs | Expected relation | Status |
|---|---|---|---|---|
| NAV-TV-001 | Ground velocity | airspeed vector + wind vector | `Vg = Va + W` | READY |
| NAV-TV-002 | Groundspeed | `Vg` | `GS = |Vg|` | READY |
| NAV-TV-003 | Track | `Vg` | `Track = atan2(Vg_y,Vg_x)` | READY |
| NAV-TV-004 | WCA | air vector + wind vector | heading correction required to obtain commanded track | READY |
| NAV-TV-005 | Drift | heading and track | signed angular difference | READY |
| NAV-TV-006 | Cross-track error | route segment + actual position | perpendicular signed distance to active segment | READY |
| NAV-TV-007 | Along-track position | route segment + actual position | projected distance along active segment | READY |
| NAV-TV-008 | ETA | remaining path + groundspeed | `ETA = remaining_distance / GS`, subject to valid temporal state | READY |
| NAV-TV-009 | Return feasibility | remaining energy + return route model | feasible only when predicted return requirement is within available energy margin | REQUIRES ENERGY MODEL |
| NAV-TV-010 | Coordinate reference | CRS/datum tagged positions | transform only when source/reference metadata are valid | REQUIRES CRS BASELINE |

## 3. Deterministic boundary cases

- Zero wind.
- Wind aligned with route.
- Wind perpendicular to route.
- Zero groundspeed / invalid velocity vector.
- Position exactly on route.
- Position before route origin.
- Position beyond route termination.
- Missing or stale navigation source.
- Invalid CRS/datum metadata.

## 4. Verification rule

Expected results are expressed as mathematical relations where the project has not yet approved numerical tolerances. Numerical tolerances remain `TBD` until established by the controlled engineering/certification basis.

A vector is not considered VERIFIED merely because the expected relation is documented. Verification requires execution, recorded result, configuration, source data, and evidence reference.

## 5. Traceability

```text
NAVIGATION_MATHEMATICAL_SPECIFICATION_001
        ↓
NAVIGATION_TEST_VECTORS_001
        ↓
NAVIGATION_VERIFICATION_MODEL
        ↓
Verification Evidence
```

No new SYS-REQ is created by this document.
