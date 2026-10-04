---
id: HMI-WORKSPACE-CONTEXT-001
type: hmi_workspace_context_routing
status: working
system: BlueSky PRO
---

# BlueSky PRO — Workspace Context Routing

## Purpose

Define the boundary between Bottom Toolbar work-context selection and the central HMI workspace.

## Controlled work contexts

- UAV
- MAP
- ADMIN
- FPV
- VIRTUAL FLT

These contexts are presentation/workspace navigation states. Selecting a context does not itself alter mission state, route calculations, telemetry acquisition, flight-control logic, safety state, or execution authority.

## Runtime chain

`BottomToolbar.activeTool → toolActivated(tool) → MainContent.workspaceContext → workspaceContextChanged(tool)`

The toolbar remains the source of the user's navigation selection. MainContent owns the current workspace context.

## Persistence

Toolbar configuration persists:

- enabled work contexts;
- context order;
- active context.

If the persisted active context is disabled or invalid, the first enabled context in the saved order becomes active.

## Current implementation boundary

The routing state is implemented and traceable. Dedicated visual workspace implementations for UAV, ADMIN, FPV and VIRTUAL FLT are not yet approved as final HMI screens.

Until those screens are approved:

- MAP remains the structural central workspace;
- existing UAV Status remains the vehicle-state surface;
- existing Left/Right panels remain operational surfaces;
- no placeholder view is treated as a final design baseline.

## Safety boundary

Workspace selection is presentation/navigation only.

`MISSION → VALIDATION → READINESS → SAFETY GATE → OPERATOR APPROVAL → EXECUTION`

No workspace-selection event may bypass this chain or issue a flight-control command.

## Verification

1. Select each enabled context.
2. Confirm MainContent.workspaceContext changes to the selected key.
3. Disable the active context.
4. Confirm automatic selection of the first enabled context.
5. Re-enable the disabled context.
6. Confirm its saved toolbar position is restored.
7. Confirm fixed LEFT / RIGHT / TIME / TOOLS elements remain fixed.
8. Confirm no mission/safety state changes are caused by context selection alone.
