---
id: HMI-DS-WORKBOOK-001
type: qt_design_studio_working_specification
status: working_reference
system: BlueSky PRO
tool: Qt Design Studio 4.8.3
target: Qt 6 + QML
---

# BlueSky PRO — Qt Design Studio Interface Work Document

## 1. Purpose

This document is the working specification for building and refining the BlueSky PRO desktop HMI in **Qt Design Studio**.

It translates the controlled HMI architecture and panel behavior into a practical visual/component workflow.

It does not replace system requirements, safety requirements, software architecture or verification records.

## 2. Controlled source order

Use the following order when making interface decisions:

1. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_DESIGN_SYSTEM.md`
2. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_PANEL_BEHAVIOR_SPECIFICATION_001.md`
3. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_SCREEN_REFERENCE.md`
4. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_PANEL_LAYOUT.md`
5. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_HMI_DESIGN_BASELINE.md`
6. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_UI_TRACEABILITY.md`
7. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_FIGMA_HANDOFF.md`
8. Controlled colors, typography, logo and asset documents.

If a visual convenience conflicts with panel behavior, panel behavior takes precedence.

## 3. Tool role

Qt Design Studio is the primary visual workspace for the QML HMI.

Use it for:
- screen composition;
- component creation;
- visual states;
- layout;
- sizing;
- spacing;
- typography;
- colors;
- transitions;
- panel visibility states;
- visual interaction prototypes;
- component preview.

Keep mission logic, safety authority, flight calculations, data acquisition and AI decision logic outside the visual design layer.

## 4. Target screen

Reference working resolution:

**1920 × 1080**

This is an ergonomic reference, not a frozen production geometry.

The design must also tolerate:
- desktop resizing;
- smaller desktop windows;
- tablet-oriented layouts where applicable;
- panel collapse;
- contextual overlays.

## 5. Canonical six-level screen

The DS working screen shall be organized as:

```
┌──────────────────────────────────────────────────────────────┐
│ TOP / HEADER PANEL                                           │
├──────────────┬───────────────────────────────┬───────────────┤
│ LEFT PANEL   │                               │ RIGHT PANEL    │
│              │        FLIGHT CHART           │               │
│ Mission      │        PRIMARY WORKSPACE      │ Checklist     │
│ Templates    │                               │ Warnings      │
│ Mission ID   │                               │ Readiness     │
│ HIDE         │                               │ Validation    │
│              │                               │ Send FPL      │
│              │                               │ Start Mission │
├──────────────┴───────────────────────────────┴───────────────┤
│ UAV STATUS / LOCAL CONTEXT                                  │
├──────────────────────────────────────────────────────────────┤
│ BOTTOM TOOLBAR / NAVIGATION                                 │
└──────────────────────────────────────────────────────────────┘
```

### Priority

1. Flight Chart / current task
2. Mission state
3. UAV state
4. C2 / link state
5. critical warnings
6. operator actions
7. secondary analysis
8. settings / diagnostics

## Mission template interaction contract

- In an existing automatic mission, show only the templates used by that mission; do not expose unrelated templates in the active mission composition list.
- In manual creation mode, show the available template catalogue and allow multi-selection.
- Each chosen template is intended to add its own scaffold to the Flight Chart; the operator combines these into one mission before submitting it to planning.
- Both paths converge on the same core planning pipeline. Only the template-selection stage differs (automatic assignment versus operator selection).
- The current Flight Chart is still a structural placeholder; visual scaffold placement and actual mission composition require the dedicated map/mission-core implementation. Do not represent the current UI selection as completed map geometry.

## 6. TOP / HEADER PANEL

Permanent, compact.

Primary content:
- BlueSky PRO identity;
- Mission Status;
- ETD;
- TOT;
- TRIP;
- ETA;
- WARNING;
- compact system/link status;
- operator context.

Rules:
- do not turn the header into a telemetry dashboard;
- do not duplicate detailed individual-UAV telemetry here;
- WARNING must visually synchronize with Right Panel warnings;
- secondary information is contextual.

### Important time semantics

- ETD = planned/estimated mission start;
- TOT = actual takeoff time;
- TRIP = mission-wide actual travel duration according to the project time model;
- ETA = estimated mission landing;
- individual UAV timing remains in UAV context.


### 6.1 Top Header composition workbench

The current Top Header visual implementation is a **working composition variant** for Qt Design Studio review.

Geometry:

`LEFT LOGO ANCHOR | ETD | TOT | TRIP | ETA | READY | WARNING | RIGHT OPERATOR ANCHOR`

Rules:
- left and right anchor fields use equal outer geometry;
- LOGO is scalable inside the left anchor;
- OPERATOR is a dedicated compact card inside the right anchor;
- the six central sectors fill the adaptive space between the two anchors;
- all six central sectors have equal width;
- the geometric center is between TRIP and ETA;
- each sector centers its heading and value on one vertical axis;
- values are placed directly below abbreviations/status headings;
- vertical separators divide the central sectors;
- Header geometry remains adjustable until visual review is complete.

Current QML components:
- `TopHeader.ui.qml`
- `HeaderSector.ui.qml`

Working composition controls include:
- header height;
- equal anchor width;
- logo scale and visual dimensions;
- operator icon/label scale;
- heading/value sizes;
- heading-to-value gap;
- separator height/width.

The working variant is documented separately in:
`BLUESKY_PRO_TOP_HEADER_COMPOSITION_WORKING_001.md`

This geometry is not frozen production geometry.

## 7. LEFT PANEL

Purpose: mission context and Mission Templates.

### Default

Show templates/tools currently used by the active task.

Other available templates remain accessible through the panel list/menu.

### Behavior

- opened by its Bottom Toolbar control;
- same control closes/collapses it;
- toolbar state follows actual panel state;
- content grows vertically;
- newly added tools/templates append at the bottom;
- existing order does not change automatically.

### HIDE

HIDE first saves the mission state.

Then:
- mission disappears from Flight Chart;
- active panel list is cleared;
- mission data is not deleted.

`+` restores the hidden mission with the same Mission ID and saved state.

### Mission ID

Format:

`BS-YYMMDD-T-NNN`

Where:
- BS = BlueSky;
- YYMMDD = creation date;
- T = M Manual / A Automatic;
- NNN = sequential number for that date.

The ID is immutable during hide, restore and editing.

## 8. CENTER / FLIGHT CHART

The map is the dominant visual area.

The DS screen must reserve maximum practical space for it.

Map operational layer may contain:
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
- Flight Profile / 3D overlays.

Map interaction:
- zoom;
- pan;
- rotate;
- tilt;
- select;
- draw polygon;
- edit route;
- edit altitude.

Do not use permanent side panels to duplicate information that belongs spatially on the map.

## 9. RIGHT PANEL

Purpose: operational action and readiness.

Default preparation state:

`CHECKLIST`

Normal hierarchy:

1. CHECKLIST
2. WARNINGS / CORRECTIONS
3. Mission Readiness
4. contextual VALIDATE MISSION
5. SEND FLIGHT PLAN
6. START MISSION

### Checklist

Example working state:

`CHECKLIST 5/8 ✓`

- completed = green;
- pending = amber;
- critical/error = red.

Completed items may be visually hidden without changing their state or audit record.

### Warning

Header WARNING and Right Panel WARNINGS / CORRECTIONS operate together.

Critical warnings may additionally appear as a large map overlay.

Closing the visual warning does not erase the event.

### Validate Mission

Do not show permanently.

Show only after a relevant change triggers automatic revalidation and confirmation is required.

Relevant changes include:
- route/WP;
- UAV/configuration;
- battery/resource;
- equipment;
- weather/forecast;
- restrictions/NOTAM;
- other readiness-affecting input.

Successful validation + required confirmation:
- show `VALIDATE MISSION`;
- use dynamic breathing green outline;
- after confirmation remove the control.

### Start Mission

Not green by default.

It becomes active only when current readiness and required checklist/validation conditions are satisfied.

## 10. UAV STATUS

Detailed current UAV telemetry belongs here, not in the aggregate header.

The UAV area should support:
- multiple UAVs;
- configurable displayed parameters;
- mission progress;
- battery/resource;
- communication state;
- current operational state.

### Local UAV decision context

When a UAV requires an operational decision:
1. warning outline appears around the UAV;
2. operator selects it;
3. local context opens;
4. telemetry and problem reason are shown;
5. system assessment is displayed;
6. concise recommendation and relevant factors are shown;
7. operator decides;
8. context collapses.

Decision controls:

- `RETURN`
- `ПРОДОЛЖИТЬ ПОЛЁТ`

RETURN must not be a permanent global Right Panel action.

The recommendation is not itself a flight command.

## 11. BOTTOM TOOLBAR

Purpose:
- panel visibility;
- workspace navigation;
- central tools;
- map tools;
- UAV access;
- workspace controls.

Rules:
- persistent;
- compact;
- state synchronized with panels;
- configurable;
- not a duplicate telemetry dashboard.

Per-panel configuration and Bottom Toolbar configuration are separate mechanisms.

## 12. Component tree for Qt Design Studio

Recommended visual decomposition:

```
App
└── Main
    ├── TopHeader
    ├── Workspace
    │   ├── LeftPanel
    │   ├── FlightChart
    │   └── RightPanel
    ├── UAVStatus
    ├── ContextOverlay
    └── BottomToolbar
```

### Core reusable components

- HeaderStatusItem
- MissionStatus
- TimeStatusItem
- WarningIndicator
- PanelFrame
- PanelHeader
- MissionTemplateItem
- MissionTemplateList
- Checklist
- ChecklistItem
- WarningItem
- ReadinessState
- ValidationAction
- FlightPlanAction
- StartMissionAction
- UAVCard
- UAVStatusItem
- UAVDecisionContext
- MapToolButton
- BottomToolButton
- BottomToolbar
- ContextOverlay

Each component should be independently previewable.

## 13. QML implementation rule

Separate visual structure from application logic.

### 13.1 Strict Qt Design Studio file boundary

**`.ui.qml` files are declarative visual files, not general-purpose JavaScript files.**

Allowed in `.ui.qml`:
- visual items, components, properties and anchors;
- static values and declarative property bindings;
- supported visual states and transitions;
- simple supported signal-handler expressions where Qt Design Studio accepts them.

**Do not put imperative JavaScript blocks in `.ui.qml`.** In particular, do not use:
- `Component.onCompleted: { ... }` blocks;
- multi-statement signal handlers such as `onClicked: { ... }`;
- local variables, loops, conditionals or function declarations embedded as behavior;
- application state mutation that requires an imperative block.

Put behavior that requires JavaScript blocks in a separate `.qml` controller/runtime component, and expose the needed state and signals to the visual `.ui.qml` component through properties and signals. Keep the visual file editable in Qt Design Studio.

When a behavior must be expressed as a single supported expression, keep it expression-only; do not expand it into a block for convenience.

### 13.2 File responsibilities

Prefer:

- `.ui.qml` — visual/component layout and declarative bindings;
- `.qml` — behavior, state transitions and imperative orchestration;
- C++ / application services — data, calculations, integration and safety-relevant logic.

Do not put flight calculations or safety authority into visual components.

### 13.3 Required validation before committing

For every edited `.ui.qml` file:
1. Check that no imperative JavaScript blocks were introduced.
2. Open the file in Qt Design Studio's 2D/visual editor.
3. Confirm the document opens without M22xx UI-file errors.
4. If logic requires a block, move it to a `.qml` file instead of suppressing or bypassing the editor error.

**Incident record:** Qt Design Studio rejected a `Component.onCompleted` JavaScript block in `MainContent.ui.qml` with error M223. The corrective rule above is now mandatory for subsequent HMI changes.

### 13.4 Applied remediation — behavior-heavy components

Imperative behavior was moved out of visual UI files into same-name runtime components:

- `BottomToolbar.ui.qml` → `BottomToolbar.qml` (persistent settings, enable/disable, reorder, activation);
- `LeftPanel.ui.qml` → `LeftPanel.qml` (mission/template actions and panel configuration);
- `PanelSettingsPopup.ui.qml` → `PanelSettingsPopup.qml` (panel-tool configuration behavior).

The visual/design counterparts remain separate where available. The QML project includes the `qml` directory, so the runtime components remain discoverable by the project.

**Validation scope:** repository scan of the `qt/BlueSkyPRO-HMI/qml` directory found no remaining imperative JavaScript block patterns in files ending in `.ui.qml` after these moves. This is a source-level check; the files must still be opened in the local Qt Design Studio to confirm editor acceptance.

## 14. States to design in DS

Every interactive component should be checked for applicable states:

- normal;
- hover;
- selected;
- active;
- disabled;
- warning;
- critical;
- success;
- hidden;
- collapsed;
- expanded;
- contextual.

Not every component requires every state.

## 15. Visual system

### Current working visual baseline

Use the controlled Design System baseline rather than arbitrary colors.

Primary visual baseline:
- background: `#050A12`
- panel surface: `#08111D`
- card surface: `#0C1725`
- selected / hover surface: `#111F30`
- primary text: `#FFFFFF`
- secondary text: `#BFBFBF`
- service text: `#7F7F7F`
- green: `#64FF00`
- amber: `#FFD339`
- red: `#FF1E14`
- cyan: `#32FFFF`
- magenta: `#FF32FF`

The separate `BLUESKY_PRO_COLORS.md` document contains an earlier/different project palette and remains draft material. Do not silently replace the working Design System palette with it.

### Typography

The current working Design System hierarchy uses:
- B612;
- B612 Mono;
- IBM Plex Sans Condensed for service information.

`BLUESKY_PRO_TYPOGRAPHY.md` remains a draft and records a different Inter / Inter Tight baseline. Final production typography is not frozen.

Therefore DS implementation should use the working Design System hierarchy while keeping the font package replaceable.

## 16. Spacing and geometry

Do not freeze arbitrary dimensions before visual review.

Use:
- consistent panel margins;
- consistent control heights;
- stable alignment grid;
- compact headers;
- restrained corner radius;
- clear separation between operational layers;
- enough map area to preserve spatial awareness.

Component dimensions should be expressed through reusable properties/tokens where practical.

## 17. What must NOT be done in DS

Do not:
- invent new operational functions;
- add permanent RETURN to Right Panel;
- make START MISSION permanently green;
- hide critical warnings through normal filtering;
- duplicate UAV telemetry into Header;
- put safety logic into UI components;
- silently change terminology;
- delete historical source material;
- introduce arbitrary semantic colors;
- make Figma/DS-only effects that cannot be transferred to QML.

## 18. DS workflow

### Phase 1 — Foundation

Build:
1. App background
2. Main workspace
3. Top Header
4. Left Panel frame
5. Flight Chart
6. Right Panel frame
7. UAV Status
8. Bottom Toolbar

### Phase 2 — Components

Build reusable components listed in section 12.

### Phase 3 — States

Implement visual states and panel visibility.

### Phase 4 — Operational screen

Assemble:
- Mission Templates;
- Flight Chart;
- Checklist;
- Warnings;
- Readiness;
- UAV status;
- Bottom Toolbar.

### Phase 5 — Ergonomic review

Check:
- map dominance;
- information density;
- panel collapse;
- warning visibility;
- readable typography;
- consistent spacing;
- desktop resizing.

### Phase 6 — QML handoff

Verify that every visual component can be represented cleanly in Qt 6 + QML without Figma/DS-specific dependencies.

## 19. Acceptance checklist

Before a screen is considered visually ready:

- [ ] six-level screen structure is preserved;
- [ ] Flight Chart remains dominant;
- [ ] Header remains compact;
- [ ] Left Panel can collapse;
- [ ] Right Panel can collapse;
- [ ] Bottom Toolbar state matches panel state;
- [ ] UAV context is separate from aggregate Header;
- [ ] RETURN exists only in local UAV context;
- [ ] START MISSION activates only at readiness;
- [ ] critical warnings remain accessible;
- [ ] secondary information is contextual;
- [ ] no duplicated operational function exists;
- [ ] typography follows the working Design System;
- [ ] semantic colors follow the working Design System;
- [ ] components are reusable;
- [ ] screen can be transferred to Qt 6 + QML.

## 20. Status

This is the **working DS document for interface construction**.

It is an implementation/design document, not a certification approval.

Final pixel geometry, final typography package and unresolved visual-baseline conflicts remain subject to explicit project approval.


## Mission review status and Create Mission button — 2026-09-28

- Mission ID text is driven by the separate `missionReviewState` property, independent of flight readiness:
  - `VERIFIED`: green `#64FF00` after assembly and checks pass.
  - `REWORK`: magenta `#FF00FF` when mission corrections are required.
  - Other/unset states fall back to the standard text color in the runtime panel.
- `MainContent.ui.qml` passes `missionReviewState` to `LeftPanel.qml`. The default is `REWORK` until a validation workflow explicitly marks the mission verified.
- The Create Mission button now uses the system card surface (`root.card`) with the green `#64FF00` label; the previous solid green fill is removed.
- Verify both runtime and Design Studio preview. Confirm the validation workflow updates `missionReviewState` to `VERIFIED` only after checks pass.


## Panel line and outline standard — 2026-09-28

- Structural panel seams and component outlines use a consistent 1 px stroke.
- Shared horizontal seams have a single owner: TopHeader draws the header/workspace boundary; BottomToolbar draws the workspace/toolbar boundary. LeftPanel and RightPanel disable their top and bottom edges in MainContent to prevent duplicate lines.
- Vertical seams are drawn once by the adjacent panel edge: LeftPanel right edge, RightPanel left and outer-right edges. The central Flight Chart does not add a competing panel frame.
- The TopHeader structural dividers use the same 1 px logical stroke as the outer frame.
- UAV cards keep a 1 px outline in every state; selection and warning are communicated by outline color, not by increasing thickness.
- Neutral internal dividers use `#7F7F7F`; structural panel seams use cyan `#32FFFF`. Semantic status colors remain reserved for status indication.
- The Flight Chart placeholder frame uses the neutral divider token rather than a near-black stroke.


## Tools configuration popup and toolbar control — 2026-09-28

- The TOOLS configuration popup sizes itself to the tool list and its content instead of using a fixed 330 × 330 rectangle. Width is based on the longest tool label with a minimum usable width; height follows the row count.
- Clicking the popup header/title area closes the configuration panel.
- The toolbar menu glyph uses the same 14 px font size as the digital clock.
- Menu glyph state colors are inverted: white while the configuration panel is expanded, cyan while collapsed.
- The clock allocation is reduced to 58 px and the menu button to 36 px, with the existing compact 6 px toolbar gap retained.
