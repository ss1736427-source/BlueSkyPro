---
id: HMI-UAV-PANEL-IMPLEMENTATION-001
type: hmi_implementation_report
status: implementation_for_qt_design_studio_review
system: BlueSky PRO
date: 2026-09-27
---

# BlueSky PRO — UAV Panel Implementation Report 001

## 1. Source documents reviewed

- `02_SYSTEM_DESIGN/INTERFACE/UAV_DISPLAY_SELECTION_001.md` — AGREED; default UAV card composition, per-UAV display configuration, optional smart tools, adaptive panel sizing.
- `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_PANEL_BEHAVIOR_SPECIFICATION_001.md` — panel state must match toolbar state; UAV-specific context; hiding is presentation-only.
- `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_PANEL_LAYOUT.md` — configurable UAV parameters; layout adapts to displayed information; configuration must not alter mission data.
- `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_QT_DESIGN_STUDIO_WORKBOOK_001.md` — UAV panel owns detailed telemetry, multiple UAVs, mission progress, battery/resource, communication state and operational state.
- `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_PANEL_TOOL_ALLOCATION_001.md` — UAV context ownership and tool/configuration boundaries.
- `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_DESIGN_SYSTEM.md` — approved colors, typography, HGT/ALT terminology and single-stroke panel boundary rule.

## 2. Default card content — corrected

The prior implementation showed `RNG` and `ETA` by default and omitted the engine operating-mode indicator. This did not match the AGREED `UAV-DISPLAY-SELECTION-001` baseline.

The default card now contains:

1. Configured aircraft/model name and registration/board ID on one line (for example, `MULTIROTOR · BS-001`).
2. Local aircraft image placeholder (until the configured local UAV/model asset is connected), without a surrounding frame.
3. Operational/readiness state and mission-state indicator.
4. Height: `HGT` below 100 m true height; `ALT` at/above 100 m, following the HMI terminology baseline.
5. Speed.
6. Battery percentage.
7. Engine operating mode: numeric percentage plus a horizontal bar.
8. Mission-state footer indicator.

`RNG` and `ETA` are optional parameters, not default permanent fields.

## 3. Optional display parameters

The per-card submenu provides these selectable fields:

- WIND — wind
- HDG — heading
- ETA — estimated arrival
- C2 — command/control link state
- CAM — camera state
- RNG — remaining range
- EET — estimated elapsed/remaining flight time as provided by the data source
- TRIP — trip time
- TOT — takeoff time
- GNSS — navigation fix/state
- LINK — communication link state
- WP — waypoint
- PROGRESS — mission progress/state
- BAT HEALTH — battery health
- PAYLOAD — payload/equipment
- TELEM — telemetry quality/state

The four default fields (height, speed, battery, engines) are configurable. Parameter order can be changed with the up/down controls. All enabled fields, including optional fields selected in the submenu, are rendered as consistent readable rows. The menu offers **ПРИМЕНИТЬ КО ВСЕМ** for copying the selected card's display configuration to the fleet.

To preserve readability, the current HMI prototype limits a card to eight displayed parameters at once. This is a presentation limit only; it does not limit telemetry acquisition or stored data.

### Adaptive card height

Card height is derived from the largest configured row count in the visible fleet. The compact four-row baseline uses 20 px metric rows and a reduced card header/image footprint. Enabling additional rows increases card height by one row increment per field. Cards remain aligned to a common height for a stable fleet grid. The dock remains anchored immediately above the bottom toolbar and overlays the map rather than replacing the map workspace. The dock's own background is fully transparent; only the individual UAV cards retain their dark card surfaces, allowing the map to remain visible between and around cards. The settings popup is taller (up to 620 px), rises above the compact dock, uses 42 px option rows for easier selection, and calculates its width from the longest option label within the available screen width.

### Mission completion indicator and remaining time

- Each card has a 0.5 px mission-completion track anchored to the card's bottom edge and spanning the full card width. The subdued track remains visible at 0% progress; the filled segment represents completion progress. Antialiasing is disabled on the line to keep its half-pixel geometry crisp in the preview. Warning state changes the fill to red; otherwise the progress fill is green.
- A numeric remaining-time label is placed at the left of the bottom indicator area and formatted as `HH:MM:SS`. The hours field is omitted when less than one hour remains, producing `MM:SS`.
- A 1000 ms QML timer decrements each card's remaining-seconds value once per second, clamped at zero. The current per-UAV countdown values and total durations are Design Studio mock data; production must bind these fields to the mission-time estimator/mission state rather than treat the prototype values as operational data.
- Mission progress is derived from the configured total and remaining seconds. When the remaining time reaches zero, the progress fill reaches 100%.


### Image treatment

The aircraft illustration area has no frame/border. Its background is transparent so the aircraft graphic reads as part of the card rather than a separate boxed tile.

### Typography

The implementation follows the working Design System hierarchy from `BLUESKY_PRO_QT_DESIGN_STUDIO_WORKBOOK_001.md`:
- `B612` for primary numeric values;
- `B612 Mono` for compact technical codes where appropriate;
- `IBM Plex Sans Condensed` for service labels and descriptive information.

Numeric values use a slightly larger size than labels to improve scanability. The final installed font availability and rendered metrics must still be verified in Qt Design Studio.



### Menu icon and engine-load scale

- The supplied three-line menu icon is reduced to 12 × 12 px and vertically centered in a 24 px header slot, matching the compact header typography; its separate 30 × 28 px click target is retained for reliable operation.
- The engine-load scale is a 2 px line placed at the bottom of the ENG row, below the value text. It no longer overlays the label or percentage.
- Engine-load color is value-dependent: 0% is neutral gray; below the cruise reference it transitions through blue; at the cruise reference it is green; above cruise it progressively transitions toward red.
- The current prototype uses a configurable 60% cruise reference (`engineCruisePercent`). This is a UI reference value, not a certified operating limit; production configuration must supply the appropriate value for each aircraft/engine profile.
- Parameter rows use 20 px height. Card height is `66 + 20 × visible rows` (subject to available panel height), and dock preferred height uses the same 20 px increment. The ENG row remains in the telemetry column; the state label is centered under the aircraft illustration in the left card column. A 2 px gray engine-load mark remains visible at 0%.

## 4. Card layout and fleet behavior

- Fleet cards are arranged in a responsive grid with a minimum target width of 280 px. The compact card height is reduced; card headings show model name and registration/board ID together on one line. Heading, label, and value font sizes adapt within defined bounds to card width. Card and dock heights adapt to the configured number of visible parameter rows.
- Cards are reordered by drag-and-drop. The order is saved using Qt Settings.
- Each UAV has its own parameter selection/order, saved by board ID.
- The selected UAV is highlighted with the approved cyan navigation color.
- A warning-state UAV receives a red outline; normal state uses the approved green semantic color.
- The fleet layout uses the available workspace between the persistent header and bottom toolbar.
- The uploaded compact three-line menu icon is represented by `icons8-menu-24.svg` and used on each card.
- Selecting a card selects the UAV. Double-click opens the existing local UAV context flow.
- The UAV toolbar button toggles the panel: first press opens it and highlights UAV; second press returns to MAP and removes the active highlight. The runtime toolbar and Design Studio toolbar presentation must stay synchronized with the actual active context.

## 5. Files

- `qt/BlueSkyPRO-HMI/qml/UAVFleetPanelForm.ui.qml` — Qt Design Studio visual form.
- `qt/BlueSkyPRO-HMI/qml/UAVFleetPanel.qml` — runtime state, per-UAV settings, persistence and drag/reorder behavior.
- `qt/BlueSkyPRO-HMI/qml/icons8-menu-24.svg` — compact submenu icon matching the supplied icon.
- `qt/BlueSkyPRO-HMI/qml/MainContent.ui.qml` — routes the UAV workspace to the new fleet panel.
- `qt/BlueSkyPRO-HMI/qml/BottomToolbar.qml` — runtime UAV toggle behavior.

The Design Studio `.qmlproject` includes QML and image directories. The CMake application explicitly packages the menu icon and all four UAV SVGs under `RESOURCES`; `BlueSkyPRO-HMI.qrc` also lists them for the qrc-based project path.

## 6. Verification status

Implemented in the controlled branch. The user's 2026-09-27 screenshots were used for iterative layout corrections. The latest correction moves the state label beneath the aircraft image, reduces the menu glyph to 12 × 12 px while retaining its click target, and makes the fleet dock background transparent. Source changes are committed; Qt Design Studio runtime/preview verification remains pending in the user's local environment.

Required DS checks:

- [ ] QML project loads without errors.
- [ ] UAV opens and closes on repeated toolbar clicks.
- [ ] UAV button highlight exactly follows panel visibility.
- [ ] Four cards are legible at the target desktop width.
- [ ] Dragging changes card order and persists after restart.
- [ ] Per-card settings toggle and reorder fields.
- [ ] Apply-to-all copies the field configuration.
- [ ] User submenu icon renders.
- [ ] Single-click selects a UAV; double-click opens its local context.
- [ ] No panel content is clipped at the tested resolution.
- [ ] Mission countdown changes once per second; hour field is omitted below 01:00:00.
- [ ] 0.5 px completion indicator spans the full card width and fill reflects mission progress.

## 7. Known integration boundary

Card values are design-preview values, not live telemetry. Production integration must bind these fields to the normalized telemetry/fleet state and the configured local UAV image. The visual layer must not invent or calculate authoritative flight state. Missing values should be represented as unavailable, and safety-critical warnings must remain visible regardless of optional-field filtering.


### Aircraft illustrations and state label

- Card status is a single label centered directly below the aircraft illustration, within the left image column, and bound to `modelData.state`; `READY` and `STBY` are mutually exclusive and replace one another. READY is green; STBY is cyan. The status is not concatenated with mission progress.
- Added local scalable SVG illustrations by aircraft class: `qml/assets/uav_multirotor.svg`, `qml/assets/uav_fixed_wing.svg`, `qml/assets/uav_heavy_multirotor.svg`, and `qml/assets/uav_vtol.svg`. They are lightweight, transparent-background schematic renders intended for the Design Studio mockup and work offline. All four mockup cards now use local assets; BS-001 and BS-003 use distinct multirotor illustrations; no card depends on an external image URL.
- Internet visual references reviewed: [white quadcopter render](https://wallpapers.com/png/white-quadcopter-dronewith-camera-58sweghtoh4bc9ab.html), [fixed-wing UAV](https://www.kindpng.com/imgv/TJmJhJ_fixed-wing-drone-png-transparent-png/), and [WingtraOne VTOL](https://www.kindpng.com/imgv/TJmomb_wingtraone-wingtra-drone-png-transparent-png/). These pages were reviewed as visual references only. The source pages do not establish a clear license for redistribution in this project, so their photos are not copied into the repository. The committed SVGs are original schematic illustrations, not copies of those images. This avoids external network dependency and uncertain image rights in the DS mockup. Obtain appropriately licensed product images before commercial release.


### Latest screenshot-driven adjustment — 2026-09-27

- `UAVFleetPanelForm.ui.qml`: the dock background color is now `transparent`; the card rectangles remain opaque so the map shows through the unused dock area without reducing card text contrast.
- The card state (`READY` / `STBY`) is centered under the aircraft image, not under the whole card or telemetry column.
- The menu glyph is 12 × 12 px and centered vertically in a 24 px header slot. The independent 30 × 28 px hit area remains unchanged.
- Preview check still required after pulling the branch; no local Qt Design Studio execution is claimed.


### Mission timer — screenshot-driven update

The QML prototype now includes per-UAV `missionTotalSeconds` and `missionRemainingSeconds`, a repeating 1000 ms countdown, `HH:MM:SS` / `MM:SS` formatting, and a full-width 0.5 px completion track with progress fill. Values are illustrative Design Studio data pending connection to authoritative mission estimates.


### Indicator visibility correction

The mission completion track is inset 2 logical pixels from the card's bottom edge so the card border cannot obscure it. It spans the full width, uses a brighter cyan-blue track, is drawn above card contents, and enables antialiasing for fractional-pixel coverage. Its height remains 0.5 logical px. Qt Quick coordinates are device-independent; actual physical-pixel coverage depends on display scaling and rasterization.


### Follow-up visibility fix

Review of the committed QML confirmed that the prior 0.5 px line was positioned exactly on the card's bottom edge. The card border could visually mask it, and antialiasing was disabled for a fractional logical-pixel height. The line is now inset from the border, uses antialiasing, and has a higher-contrast track color. Preview verification is still required.


### Countdown-row alignment update

The 0.5 px mission-progress indicator now occupies the same horizontal row as the remaining-time label. The label has a fixed 16 px row height; the indicator is positioned from the label's vertical center and begins to its right, extending to the card's right inset. The time text and progress line therefore share one baseline row without overlapping.


### Single-color progress indicator

The countdown row now renders only one green 0.5 px mission-progress line. The cyan background track and nested fill were removed; the green line's length is proportional to mission completion. No second indicator line is rendered.


### Remaining-time indicator direction

Before mission start, the green indicator is full width (remaining time equals the mission's maximum duration). As the countdown decreases, the green line contracts from its right edge toward the left. At 0 remaining seconds, the line has zero width. The numeric label counts down from the configured maximum mission duration to `00:00` (hours omitted below one hour).


### Countdown start condition

The countdown is held at the configured maximum before departure. Each UAV model has `missionStarted: false` initially; the 1-second timer decrements `missionRemainingSeconds` only after `missionStarted` becomes `true`. The mission-start workflow must set this flag at actual departure/start-of-mission. In the current Design Studio mock, no flight-start control is wired, so the countdown remains at its maximum until connected to that event.


### Right-panel symmetry and shared seams — 2026-09-27

- `RightPanel.ui.qml` now uses the same `#08111D` surface as `LeftPanel.qml`, with matching cyan (`#32FFFF`) outer outline and `#7F7F7F` internal divider tone.
- `MainContent.ui.qml` binds `rightWidth` to `leftWidth`, so both side panels use the same width.
- Right-panel top and bottom borders are disabled in the main composition. The full-width TopHeader bottom stroke and BottomToolbar top stroke own those shared seams, matching the left-panel arrangement and avoiding doubled horizontal lines.
- The right panel retains its left and right vertical cyan edges. Confirm final appearance in Qt Design Studio Preview after pulling the branch.


### Adaptive fleet-card layout between side panels — 2026-09-27

- `UAVFleetPanel` is anchored between `leftPanel.right` and `rightPanel.left`, so its available width follows the central workspace rather than the full application window.
- Fleet cards are laid out in responsive columns. A single card is centered in the available space. With multiple cards, each row is centered as a group around the workspace center; an incomplete final row is centered independently.
- Card width adapts to the available space and column count, with a 240 px target minimum and 420 px maximum. When the workspace is narrower than the target minimum, cards shrink to fit rather than overflow.
- The layout recalculates its column count and card dimensions as the available workspace changes. Preview verification at narrow, medium, and wide window sizes remains required.


### Fleet layout anchor correction — 2026-09-27

The previous preview showed fleet-card content stacked at the far left because `UAVFleetPanel` was a root-level item attempting to anchor to `LeftPanel` and `RightPanel`, which are children of `workspace`. Those items were not valid anchor siblings, so the intended horizontal constraints did not resolve. `UAVFleetPanel` has now been moved inside `workspace`, alongside both side panels. Its left/right anchors now target sibling panels, and it is vertically centered in the workspace. This corrects the coordinate-space/anchor issue behind the misaligned preview.


### UAV panel bottom docking and side-panel collapse — 2026-09-27

- The fleet panel is bottom-anchored to the workspace, whose lower edge is the top of the bottom toolbar. Its card strip therefore stays immediately above the toolbar instead of vertically covering the central map.
- Activating the UAV tool explicitly sets both `leftPanelOpen` and `rightPanelOpen` to false. The side panels collapse and the fleet view receives the full workspace width.
- Selecting another tool does not automatically reopen the side panels; the user can reopen them with the fixed LEFT/RIGHT toolbar controls.
- Verify the resulting placement and panel toggling in Qt Design Studio Preview.


### Preserve side panels in UAV mode — 2026-09-27

- Activating the UAV tool changes the workspace mode but does not close the left or right side panel.
- Side-panel visibility is controlled only by the respective `leftPanelOpen` and `rightPanelOpen` states, not by the active tool.
- The fleet cards remain constrained to the central area between the side panels; the available card width therefore reflects whether either side panel is open.
- Verify in Qt Design Studio Preview that LEFT and RIGHT remain visible when UAV is active and still respond to their own toolbar toggles.


### Selectable UAV cards and movable parameter panel — 2026-09-27

- Clicking a UAV card selects it; the selected card is shown with the existing cyan selection outline. Opening that card's settings also selects it.
- The parameter panel reads and edits the configuration for the currently selected UAV. Changing selection while the panel is open retargets the settings to the newly selected UAV. The explicit “apply to all” action remains separate.
- The parameter panel can be dragged by its title/header area. Dragging is constrained to the workspace bounds.
- The last panel position is persisted through `Qt.labs.settings` and reused on subsequent openings and application launches.
- The fleet overlay now occupies the full central workspace while the card grid remains bottom-anchored. This gives the settings panel a workspace-sized coordinate area without changing the card strip placement.
- Verify card selection, per-UAV parameter isolation, drag bounds, and position persistence in Qt Design Studio / application runtime.


### Automatic mission highlight — 2026-09-27

- Automatically generated missions are identified by the `-A-` segment in the mission ID (for example, `BS-260920-A-001`).
- The mission row outline uses `#64FF00` for automatic missions.
- Other mission types retain the standard divider outline.
- The highlight is derived from the mission ID, so it updates when the mission ID changes.
