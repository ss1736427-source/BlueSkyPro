---
id: HMI-BOTTOM-TOOLBAR-CONFIG-001
type: hmi_bottom_toolbar_configuration
status: working_reference
system: BlueSky PRO
implementation: qt/BlueSkyPRO-HMI/qml/BottomToolbar.ui.qml
controlled_branch: feat/hmi-controlled-cleanup-001
---

# BlueSky PRO — Bottom Toolbar Configuration

## 1. Purpose

This specification defines the controlled behavior of the Bottom Toolbar configuration layer.

The Bottom Toolbar is a persistent HMI navigation surface. Its configuration changes presentation and navigation only. It does not modify mission data, route calculations, telemetry acquisition, flight-control logic, safety state or execution authority.

## 2. Toolbar allocation

The toolbar contains three fixed system elements and one configuration control:

- LEFT ◀ — fixed left-panel visibility control;
- configurable work contexts;
- RIGHT ▶ — fixed right-panel visibility control;
- TIME — fixed system time display;
- ☰ TOOLS — fixed configuration control.

Configurable work contexts are:
- UAV
- MAP
- ADMIN
- FPV
- VIRTUAL FLT

TOOLS is not a work context and is never included in the configurable work-context list.

## 3. Configuration model

The configuration model contains two independent persisted properties:
1. Order — ordered list of all configurable work contexts.
2. Enabled state — enabled/disabled state for each work context.

The implementation keeps the complete ordered model even when a context is disabled. The toolbar presentation filters disabled contexts without deleting them from the model.

Default order: UAV → MAP → ADMIN → FPV → VIRTUAL FLT
Default state: all five contexts enabled.

Unknown, duplicated or missing persisted entries are normalized against the controlled default set. Missing contexts are restored to the model.

## 4. Enable / disable behavior

The TOOLS configuration surface uses ☑ / ☐ controls.

When a context is disabled:
- it disappears from the visible Bottom Toolbar;
- its model entry remains present;
- its saved position remains unchanged;
- mission data and system capabilities remain unchanged.

When a context is re-enabled:
- it returns to its previously saved position;
- it does not get appended merely because it was temporarily disabled.

At least one configurable work context remains enabled. This prevents the central workspace from entering a state with no available work context.

## 5. Active-context consistency

The active work context must always be one of the currently enabled contexts.

If the active context is disabled:
1. the disabled context is removed from the visible toolbar;
2. the system selects the first enabled context in the persisted order;
3. the active-context state is persisted.

This maintains: Toolbar Composition ↔ Active Context ↔ Workspace Context

No unavailable context may remain selected.

## 6. Reordering

Configurable work contexts may be reordered through the TOOLS configuration surface.

Reordering:
- changes only the persisted presentation order;
- does not change tool availability;
- does not alter mission logic;
- does not change safety authority;
- persists immediately after the controlled reorder operation.

Fixed elements LEFT, RIGHT, TIME and TOOLS cannot be reordered.

## 7. Persistence

Configuration is persisted using the Qt settings mechanism.

Persisted state includes:
- ordered work-context list;
- enabled/disabled composition;
- active work context.

On application start the persisted state is loaded, normalized against the controlled tool definition, checked for active-context consistency, and applied to the toolbar.

If stored data is invalid, the controlled default state is used.

## 8. Separation from panel configuration

Bottom Toolbar configuration is distinct from per-panel tool configuration.

Bottom Toolbar configuration controls which work contexts are presented, their order, and the active work context.

Panel configuration controls which information blocks/tools are displayed inside an individual panel.

Neither configuration layer deletes underlying information or changes mission/control logic merely by changing presentation.

## 9. Safety boundary

The configuration layer cannot issue a flight command, bypass readiness, bypass validation, bypass the Safety Gate, authorize execution, or alter mission calculations.

The HMI chain remains:
MISSION → VALIDATION → READINESS → SAFETY GATE → OPERATOR APPROVAL → EXECUTION

Critical safety information remains governed by the applicable safety and warning layers and cannot be made unavailable merely through toolbar filtering.

## 10. Controlled implementation

Implementation file: qt/BlueSkyPRO-HMI/qml/BottomToolbar.ui.qml

Controlled implementation commits:
- 8c6b5687ef3b8c8650caf90404d22dec161c1f95 — persisted TOOLS configuration, enabled composition, order and active-context handling.
- de788a344364c4bbba3f2f101289d6e7f903ff9c — reorder-state stabilization and toolbar clock refresh.

This document records the intended Design System / HMI behavior represented by those implementation changes.

## 11. Verification targets

- [ ] disabling a non-active context removes only its toolbar presentation;
- [ ] re-enabling restores its saved position;
- [ ] reordering persists after application restart;
- [ ] enabled/disabled composition persists after application restart;
- [ ] active context persists when still enabled;
- [ ] disabling the active context selects the first enabled context;
- [ ] the last enabled work context cannot be disabled;
- [ ] fixed elements remain fixed;
- [ ] no DRAG TO REORDER item exists;
- [ ] panel configuration remains independent from Bottom Toolbar configuration;
- [ ] no mission/safety state changes are caused by presentation configuration.

## 12. Status

Working Design System reference aligned with the controlled HMI implementation. Final production geometry, typography and certification status remain governed by their respective controlled specifications.

### Centered toolbar tools and menu icon — 2026-09-27

- Replaced the visible `TOOLS` text label with a compact hamburger/menu glyph (`☰`); the existing button continues to open/close the tool configuration popup.
- LEFT and RIGHT remain fixed anchor controls. The clock and menu button remain in the right-side group.
- The enabled tool buttons are centered within the available span between the LEFT and RIGHT anchors. Their row width follows the visible tool delegates, so the group re-centers when tools are enabled/disabled or added.
- Existing tool ordering, drag-to-reorder, and persisted configuration are retained.
- Verify in Qt Design Studio at the target window size and with all available tools enabled; very narrow windows may require a separate overflow/scroll treatment.


### Odd-count center alignment — 2026-09-27

- When the visible tool count is odd, the middle tool button is anchored by its own center to the exact center of the available toolbar tool area.
- Remaining buttons are laid out outward to the left and right, preserving their order and spacing.
- For an even number of tools, the complete group remains centered as a group.
- Position calculations use each rendered button's actual delegate width (with a label-based fallback before delegates are available), avoiding spacing drift from estimated text widths. Verify in Qt Design Studio after drag-reordering and when the visible tool set changes.


### Virtual Flight centered layout and stable spacing — 2026-09-27

- The five-tool default order is now MAP, ADMIN, VIRTUAL FLT, FPV, UAV, placing VIRTUAL FLT in the exact center with two tools on each side.
- Known previous default/displayed orders are migrated to this centered arrangement; custom user orders are preserved.
- Toolbar tool buttons use a uniform 100 px width and a consistent 6 px gap. This removes spacing drift caused by label-dependent widths during and after drag-reordering.
- Verify in Qt Design Studio: centered VIRTUAL FLT with five tools, drag each tool across the group, and confirm equal gaps after release.


### Center reference — outer panel width — 2026-09-27

- The tool group center is calculated from the full outer toolbar panel width, not from the remaining area between LEFT and RIGHT anchors.
- The full-panel center is converted into the tool area's local coordinate system so anchors remain fixed without shifting the center reference.
- Odd-count layouts place the middle button's center at the panel center; even-count layouts center the full group around the panel center.
