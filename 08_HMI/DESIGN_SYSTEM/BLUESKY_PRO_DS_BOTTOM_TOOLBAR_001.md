---
id: DS-BOTTOM-TOOLBAR-001
type: design_system_component
status: working_reference
system: BlueSky PRO
---

# BlueSky PRO тАФ DS Bottom Toolbar 001

## Purpose

Canonical Design System component for persistent workspace navigation and panel control.

## Structure

`LEFT тЧА | UAV | MAP | ADMIN | FPV | VIRTUAL FLT | RIGHT тЦ╢ | HH:MM | тШ░`

### Fixed elements
- LEFT тЧА
- RIGHT тЦ╢
- TIME тАФ current clock in `HH:MM`
- тШ░

### Configurable workspace contexts
- UAV
- MAP
- ADMIN
- FPV
- VIRTUAL FLT

Fixed elements never participate in workspace reordering.

## Visual rules

- Main background: `#050A12` (controlled Design System source of truth)
- Toolbar/panel surface: `#08111D`
- Button/card surface: `#0C1725`
- Active/selected surface: `#111F30`
- Primary text: `#FFFFFF`
- Service text/borders: `#7F7F7F`
- Active workspace accent: `#32FFFF`
- 1 px service/component borders on workspace controls. The outer toolbar panel frame uses the same `#32FFFF` / 1 px structural stroke as the Top Header.
- TIME and тШ░ system controls are frameless; the vertical divider before TIME remains the structural separator and uses the same `#32FFFF` / 1 px structural stroke as the Top Header.
- Reference DS geometry: `1440 ├Ч 96`.
- Compact persistent height; application geometry remains responsive to the host window.
- Active context is visually distinct but does not become a command/flight-control state.
- Working typography: B612 Mono Bold 16 px for toolbar labels; IBM Plex Sans Condensed 9 px for DS annotation/reference text.
- TIME is a fixed system element. The runtime displays the current clock in `HH:MM` format. The DS visual reference uses `HH:MM` as the format placeholder and must not contain a fabricated clock value.
- RIGHT is right-anchored against the system-control zone with a 16 px gap before the structural divider. Reference geometry places RIGHT at x=1134, divider at x=1258, TIME at x=1274 and тШ░ at x=1362. TIME and тШ░ retain their compact widths but have no component background; only their text/icon remains visible.

## Qt Design Studio source

The editable Qt Design Studio visual source is:

`qt/BlueSkyPRO-HMI/qml/BottomToolbar.Design.ui.qml`

It is intentionally a visual-only `.ui.qml` component:
- no runtime JavaScript functions;
- no Settings persistence;
- no mission/flight logic;
- no safety authority;
- no telemetry control.

Runtime behavior remains in:

`qt/BlueSkyPRO-HMI/qml/BottomToolbar.ui.qml`

The Design Studio project entry point is:

`qt/BlueSkyPRO-HMI/BlueSkyPRO-DesignStudio.qmlproject`

The two files are intentionally separated so Qt Design Studio can edit the visual component without moving application behavior into the visual layer.

## Interaction rules

1. Workspace contexts may be enabled or disabled.
2. Their order is persistent.
3. Disabling the active context immediately selects the first enabled context in saved order.
4. At least one workspace context must remain enabled.
5. Re-enabled contexts return to their saved position.
6. тШ░ configures the workspace toolbar; it is not a workspace context.
7. LEFT/RIGHT control panel visibility and remain fixed.
8. TIME is a system element and remains fixed.
9. Toolbar configuration changes presentation/navigation only.
10. **Strict enabled-button invariant:** if a workspace context is ENABLED in TOOLS CONFIGURATION, its button MUST be present on the Bottom Toolbar. Reordering, dragging, clicking, persistence restore, or delegate/model refresh MUST NOT remove an enabled button.
11. Only an explicit DISABLE action may remove a workspace button from the Bottom Toolbar.
12. Reordering changes position only; it never changes the enabled/disabled state.
13. If an internal model/projection inconsistency occurs, the runtime must restore missing enabled buttons before completing the reorder operation.
14. Toolbar configuration cannot delete data, alter mission logic, stop telemetry, change route calculations or bypass safety authority.

## Safety boundary

`MISSION тЖТ VALIDATION тЖТ READINESS тЖТ SAFETY GATE тЖТ OPERATOR APPROVAL тЖТ EXECUTION`

The Bottom Toolbar is navigation/presentation infrastructure and cannot bypass this chain.

## Visual reference

`BLUESKY_PRO_DS_BOTTOM_TOOLBAR_001.svg`

This is a DS reference component, not a final application screenshot and not a certification artifact.
