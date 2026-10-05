---
id: HMI-PANEL-BEHAVIOR-001
type: hmi_panel_behavior_specification
status: working_reference
system: BlueSky PRO
source: 2026-09-19 controlled HMI consolidation
---

# BlueSky PRO — Panel Behavior Specification

## 1. Scope

This document consolidates the panel behavior established during the 2026-09-19 HMI work. It defines behavior, not final pixel geometry.

The screen is treated as six functional levels:

1. TOP / HEADER PANEL
2. LEFT PANEL
3. CENTER / FLIGHT CHART
4. RIGHT PANEL
5. UAV STATUS / LOCAL CONTEXT
6. BOTTOM TOOLBAR / NAVIGATION

The Center / Flight Chart is the primary workspace and is not a conventional side panel.

## 2. Global panel behavior

### 2.1 Independent components

Each panel is an independent UI component with:
- stable UI-ID;
- purpose;
- displayed information;
- states;
- actions;
- min/preferred/max size;
- hide/show rules;
- role applicability;
- related requirements.

### 2.2 Visibility states

Supported conceptual states:
- OPEN / EXPANDED
- COLLAPSED / HIDDEN
- CONTEXTUAL
- TEMPORARILY SURFACED

Hiding changes presentation only. It does not delete data, stop acquisition, alter calculations, or change mission state.

### 2.3 State synchronization

A panel-control button must always represent the actual panel state.

`Panel State ↔ Toolbar State ↔ Workspace State`

A panel must not be visually closed while its control indicates OPEN, or vice versa.

### 2.4 Configuration

Panel configuration controls which supported information blocks/tools are displayed inside that panel.

- Configuration is independent per panel.
- Hidden tools remain available.
- Enabling a tool does not change mission logic.
- Newly enabled tools follow the panel's established layout order.
- Configuration persists for the operator/device context.
- Safety-critical warnings cannot be made inaccessible by filtering.
- Required critical states may be surfaced temporarily regardless of normal filtering.

Panel configuration is distinct from Bottom Toolbar configuration.

### 2.5 Adaptive sizing

The container adapts to its content within defined min/preferred/max bounds.

For the Left Panel:
- growth is vertical, top-to-bottom;
- newly added tools/templates are appended at the bottom;
- existing order is not changed automatically;
- adaptation changes the container size, not the established visual style of individual items.

### 2.6 Map priority

When side panels are hidden or collapsed, the released space belongs to the Flight Chart. The map remains the dominant working area.

## 3. TOP / HEADER PANEL

### Role

Permanent compact aggregate mission/system status.

### Behavior

- remains visible during normal operation;
- stays compact;
- presents mission-wide state and key time parameters;
- secondary information is contextual rather than permanently crowded into the header;
- warning state in the header is synchronized with the Right Panel warning state.

### Functional content

The working architecture includes:
- BlueSky PRO identity;
- Mission Status;
- ETD;
- TOT;
- TRIP;
- ETA;
- WARNING;
- compact system/link state as applicable;
- operator context.

Individual UAV telemetry is not duplicated into the aggregate mission header.

## 4. LEFT PANEL

### Role

Mission context and Mission Templates.

### Behavior

- opened by the assigned Bottom Toolbar control;
- same control closes/collapses it;
- toolbar control state follows actual panel state;
- panel remains compact and content-sized;
- default list shows templates currently used by the active task;
- other available templates remain accessible through the existing list/menu;
- selecting a hidden template makes it visible and active/highlighted;
- new tools/templates are appended at the bottom.

### Task-first template selection

Mission creation begins with the operator stating the task in the task-description field. The operator does not have to choose a template before describing the task.

As the task is entered or refined, the system identifies the mission templates relevant to the requested work and automatically surfaces/highlights those templates in the Left Panel. This is a contextual recommendation/selection state, not silent execution of a mission.

Rules:
- The operator's task statement is the input; template matching is derived from the task.
- All applicable templates may be highlighted when a task combines related work types.
- Highlighting must distinguish templates that are applicable to the current task from templates that are merely available.
- The operator can inspect, add or remove applicable templates before applying the mission configuration.
- The system must not silently discard a relevant template or commit a mission configuration without operator confirmation.
- If the task is ambiguous or no confident match exists, show the uncertainty and let the operator choose or clarify the task.
- Once confirmed, the selected templates provide the corresponding planning algorithms/parameters; the mission remains one coherent mission context.
- Re-evaluate the highlighted set when the task description changes, preserving explicit operator choices where they remain compatible and indicating any conflicts.
- The full template catalogue remains accessible; contextual highlighting does not hide or delete templates.

### Mission context

`HIDE`:
- automatically saves current mission state first;
- hides the current mission from Flight Chart;
- clears the active panel list;
- deletes no data.

`+`:
- restores a hidden mission;
- restores the saved state and the same Mission ID;
- is a mission-context restore mechanism, not a duplicate-ID mechanism.

Delete remains a separate explicit operation.

### Mission ID

Created immediately after mission creation.

Format:
`BS-YYMMDD-T-NNN`

- `BS` — BlueSky
- `YYMMDD` — date
- `T` — M Manual / A Automatic
- `NNN` — sequential number for that date

Mission ID is immutable through save, hide, restore and editing. Manual corrections to an automatic mission do not change A to M.

### Close behavior

Left Panel closes through:
- its Bottom Toolbar button;
- double-click in the map area;
- automatic closure at mission start.

A single map click does not close it.


### Approved interaction decision — task-first template activation (2026-09-30)

**Decision:** The operator states the task first. BlueSky PRO identifies the applicable mission templates and highlights them in the Left Panel as the task is entered or refined.

**Approved interaction sequence:**
1. Operator enters the task in the task-description field.
2. The system matches the task to one or more relevant templates.
3. Applicable templates are surfaced and highlighted; unrelated templates remain available in the full catalogue but are not emphasized.
4. If the task combines several work types, all applicable templates may be highlighted together.
5. Operator reviews the proposed set and may add or remove templates.
6. On operator confirmation, the selected templates provide the planning algorithms and parameters for one coherent mission.
7. If the task changes, the system updates the highlighted set, preserves compatible explicit choices, and flags conflicts or ambiguity.

**Interaction constraints:**
- Do not require the operator to select a template before describing the task.
- Highlighting is a recommendation/selection state, not mission execution.
- Do not silently discard a relevant template or commit the configuration without confirmation.
- If matching is ambiguous or confidence is insufficient, show that uncertainty and request clarification or operator selection.
- Keep the complete template catalogue accessible; contextual highlighting must not hide, delete, or reorder templates.
- Preserve the existing compact Left Panel and map-first workspace. This decision changes interaction behavior only; it does not introduce a new module or alter system architecture.

**Acceptance criteria:**
- A task description can be entered before any template is selected.
- The applicable template set updates when the task text changes.
- Multiple applicable templates can be highlighted for a combined task.
- The operator can review and adjust the set before confirmation.
- Ambiguous input is surfaced rather than silently mapped.
- The confirmed templates remain part of one mission context and invoke their associated planning logic.


## 5. CENTER / FLIGHT CHART

### Role

Primary workspace.

### Behavior

The Flight Chart receives the maximum practical workspace and visually dominates the screen.

Operational spatial information belongs here:
- route;
- waypoints;
- UAV positions;
- restrictions / zones;
- NOTAM;
- weather/wind;
- active/optimal route;
- completed/uncompleted route;
- mandatory waypoints;
- spatial warnings;
- 2D / Terrain / 3D and Flight Profile overlays where applicable.

Map controls and route editing remain spatial operations, not replacements for panel content.

## 6. RIGHT PANEL

### Role

Operational control, checklist, warnings and readiness.

Detailed current telemetry belongs to the UAV Panel. Spatial operational information belongs to the map. Non-immediate technical/history/archive information belongs to Administration / Technical State / Logs.

### Normal action hierarchy

1. CHECKLIST
2. active WARNINGS / CORRECTIONS
3. Mission Readiness
4. contextual VALIDATE MISSION after automatic re-check
5. SEND FLIGHT PLAN
6. START MISSION when fully ready

RETURN is not a normal Right Panel action.

### Warning behavior

Header WARNING and Right Panel WARNINGS / CORRECTIONS operate in parallel.

When a warning appears:
1. Header WARNING changes state.
2. Right Panel warning area highlights.
3. The relevant warning expands automatically.
4. Required/recommended action is shown.

Closing the visual warning does not erase the event.

Serious/critical warnings may also appear as a large alert over the map. The event remains in the Journal/Audit trail.

### Automatic validation

Relevant readiness-affecting changes trigger automatic revalidation, including route/WP, UAV/configuration, battery/resource, equipment, weather/forecast, restrictions/NOTAM and other relevant inputs.

When validation succeeds and operator confirmation is required:
- `VALIDATE MISSION` appears;
- its outline uses the defined dynamic breathing green treatment;
- after confirmation it stops and disappears.

When validation fails, the corresponding warning/error is shown.

### Start Mission

`START MISSION` is not green by default. It becomes active only when current readiness and required checklist/validation conditions are satisfied. A readiness-invalidating change returns it to the non-ready/disabled state.

## 7. UAV STATUS / LOCAL CONTEXT

### Role

Current state and decision context of individual UAVs.

The UAV Panel carries detailed current telemetry and mission progress that should not be duplicated in the aggregate header.

When a specific UAV requires an operational decision:
1. warning outline appears around the affected UAV/card;
2. operator selects the UAV;
3. local context opens;
4. telemetry and problem reason are shown;
5. system performs preliminary assessment;
6. concise recommendation with relevant reasoning factors is shown;
7. operator decides;
8. local context collapses.

Local decision options:
- `RETURN`
- `ПРОДОЛЖИТЬ ПОЛЁТ`

The system recommends; the operator decides. The recommendation is not itself a flight command.

The assessment may consider telemetry, detected deviation, battery/resource, remaining distance/time, route and mission state, weather, communications, feasibility of return/continuation and mission-completion impact.

If redistribution is required, it is a separate controlled mission operation and is not silently performed by the recommendation layer.

Journal/Audit records the event, assessment factors, recommendation, operator decision and resulting action.

## 8. BOTTOM TOOLBAR / NAVIGATION

### Role

Persistent workspace/tool navigation and panel control.

### Behavior

- controls panel visibility;
- provides access to configured tools;
- reflects actual panel state;
- uses a distinct configuration mechanism from per-panel tool configuration;
- maintains the established tool order unless explicitly reordered;
- does not become a duplicate telemetry dashboard.

## 9. Overlay / Context behavior

Context panels are used for local decisions and secondary information.

They:
- appear only when context requires them;
- preserve the underlying mission state;
- collapse after completion of the contextual interaction;
- do not silently change mission state unless an explicit operator action and controlled operation do so.

## 10. Safety boundary

The HMI represents:

`MISSION → VALIDATION → READINESS → SAFETY GATE → OPERATOR APPROVAL → EXECUTION`

HMI, planning, optimization, AI and simulation do not bypass safety authority.

## 11. Status and source boundary

This is a working reference derived from the controlled 2026-09-19 HMI commits. It is not a certification approval and does not freeze final geometry, typography or pixel dimensions.
