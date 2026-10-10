---
id: DS-PANEL-MASTER-001
type: design_system_component
status: working_reference
system: BlueSky PRO
---

# BlueSky PRO — DS Panel Master 001

## Purpose

Reusable panel master for the BlueSky PRO Design System. It defines the common visual and behavioral skeleton used by operational panels without freezing a specific screen.

## Anatomy

1. **Panel frame** — compact black surface with 1 px service border.
2. **Panel header** — title + contextual status.
3. **Section header** — uppercase functional grouping.
4. **Data row/card** — primary value, secondary explanation and state.
5. **Warning / Corrections block** — amber/red state surface; closing it does not erase the event.
6. **Action group** — explicit operator actions; disabled actions remain visible when useful.
7. **State footer** — documents supported panel states and safety boundary.

## Working visual tokens

| Token | Value |
|---|---|
| Background | #000000 |
| Panel surface | #050505 |
| Primary | #FFFFFF |
| Secondary | #BFBFBF |
| Service | #7F7F7F |
| Ready | #64FF00 |
| Warning | #FFD339 |
| Critical | #FF1E14 |
| Information | #32FFFF |
| Accent | #FF32FF |

Typography follows the current HMI reconciliation baseline: B612 / B612 Mono for operational data and IBM Plex Sans Condensed for service information.

## Behavioral rules

- Supported states: OPEN / EXPANDED, COLLAPSED / HIDDEN, CONTEXTUAL, TEMPORARILY SURFACED.
- Hiding changes presentation only; it does not delete data, stop acquisition, alter calculations or change mission state.
- Panel state, toolbar state and workspace state must remain synchronized.
- Panel configuration is independent from Bottom Toolbar configuration.
- Safety-critical warnings cannot be made inaccessible by filtering.
- Actions that can affect mission execution remain subject to the safety chain:
  MISSION → VALIDATION → READINESS → SAFETY GATE → OPERATOR APPROVAL → EXECUTION.
- AI recommendations are advisory and never replace operator approval.

## Variants

The master is intended to support:
- LEFT / Mission Context
- RIGHT / Operational Control
- UAV / Current State
- ADMIN / Technical & Administrative State
- FPV / FPV Context
- VIRTUAL FLT / Simulation Context

Variant-specific content is defined by the corresponding workspace specification; the master does not invent those screens.

## Reference

Visual reference: `BLUESKY_PRO_DS_PANEL_MASTER_001.svg`.

This is a Design System reference component, not a certification artifact and not a final pixel-locked implementation.
