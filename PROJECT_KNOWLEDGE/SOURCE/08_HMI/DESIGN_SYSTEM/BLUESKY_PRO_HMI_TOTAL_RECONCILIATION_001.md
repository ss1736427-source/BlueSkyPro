---
id: HMI-RECONCILIATION-001
type: hmi_reconciliation
status: working_reconciled_reference
system: BlueSky PRO
---

# BlueSky PRO — HMI Total Reconciliation

## Purpose

This document records the source set used to rebuild the current desktop HMI reference. It does not silently convert draft material into approved requirements.

## Source hierarchy used

1. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_DESIGN_SYSTEM.md` — visual source of truth; current working baseline.
2. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_PANEL_BEHAVIOR_SPECIFICATION_001.md` — consolidated panel behavior and interaction rules from the 2026-09-19 controlled HMI changes.
3. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_PANEL_LAYOUT.md` — panel classes, map priority, layout flexibility and UAV-panel configuration.
4. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_SCREEN_REFERENCE.md` — detailed operational Right Panel behavior and local UAV decision context.
5. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_HMI_DESIGN_BASELINE.md` — component model, UX priorities and layout areas.
6. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_UI_TRACEABILITY.md` — UI IDs, functional allocation and safety boundary.
7. `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_FIGMA_HANDOFF.md` — Figma componentization and Qt 6 + QML handoff rules.
8. `02_ARCHITECTURE/UX/BLUESKY_PRO_UX_MINIMAL_INFORMATION_PRINCIPLE_001.md` — minimal information density and contextual display.
9. `02_SYSTEM/Design/Interface/Mission Dashboard.md` — earlier UAV/panel display configuration material; treated as historical supporting material where it is not superseded.
10. `08_HMI/DESIGN_SYSTEM/FIGMA/BlueSky_PRO_Figma_Visual_Tokens.json` and `.svg` — visual token reference.
11. `08_HMI/ASSETS/logo/README.md` and `08_HMI/DESIGN_SYSTEM/BLUESKY_PRO_LOGO.md` — controlled logo asset rules.
12. Mission, navigation, map and requirements/traceability documents were checked for UI-impacting allocation and terminology.

## 2026-09-19 panel consolidation

The panel behavior specification is the canonical consolidation of the following controlled commits: `257ed2be`, `0aea2f39`, `36748919`, `9fde6b9c`, `438cf15`, `954c50ba`, `c2b9d1dc`, `baf999d2`, `65df3ea6`, `2640be05`, `c079957f`. These changes are retained in Git history and are not treated as disposable drafts.

## Reconciled desktop layout

- Header: compact persistent aggregate mission/system status.
- Left Panel: mission context and compact Mission Templates; secondary analysis/instruments/ATC/diagnostics are expandable/contextual.
- Center: Flight Chart is the primary workspace.
- Right Panel: checklist, active warnings/corrections, readiness, contextual validation, flight-plan transfer, Start Mission.
- UAV Status: individual UAV sections, configurable displayed parameters, persistent current UAV context.
- Bottom Navigation: panel state controls and configurable central tools; not a duplicate telemetry panel.
- Overlay/Context Panel: local UAV decision context and other contextual operations.
- Critical warnings remain accessible regardless of display filtering.
- Hidden information remains available to the system; hiding changes presentation only.

## Fixed interaction allocations

- RETURN is not a permanent Right Panel action. It appears in the affected UAV local context.
- START MISSION is active only at full current readiness.
- VALIDATE MISSION is contextual after automatic revalidation and operator confirmation is required.
- Mission Analysis, Instruments, ATC details and extended diagnostics are not forced into permanent high-density presentation.
- Map remains visually dominant.
- Individual UAV data is not duplicated into the aggregate mission header.

## Visual baseline used for the reference screen

The current working visual source of truth specifies:

- background: `#050A12`;
- panel surface: `#08111D`;
- card surface: `#0C1725`;
- selected / hover surface: `#111F30`;
- primary text: `#FFFFFF`;
- secondary text: `#BFBFBF`;
- service text: `#7F7F7F`;
- green: `#64FF00`;
- amber: `#FFD339`;
- red: `#FF1E14`;
- cyan: `#32FFFF`;
- magenta: `#FF32FF`.

Typography working baseline:

- B612 Mono Bold 36 px — primary flight data;
- B612 Bold 22 px — main headers;
- B612 Mono Bold 20 px — status/numeric zones;
- B612 Mono Bold 20 px — navigation;
- B612 Mono Bold 20 px — target/selected;
- B612 Bold 16 px — section headers;
- B612 13 px — labels;
- B612 Mono 13 px — secondary numeric;
- IBM Plex Sans Condensed 12/11 px — service information.

## Explicit unresolved source conflict

`BLUESKY_PRO_TYPOGRAPHY.md` remains `DRAFT_FOR_AGREEMENT` and records Inter / Inter Tight for general UI with B612/B612 Mono for telemetry. The working visual source of truth, design-system token board and approved hierarchy use B612/B612 Mono/IBM Plex Sans Condensed.

The reference screen therefore uses the **working Design System hierarchy** and does not claim that the final font package has been approved. This conflict must be resolved before final production typography is frozen.

## Geometry status

The 1920×1080 reference is a working ergonomic reference, not a final approved fixed layout. Panel components retain the min/preferred/max and independent configuration model from the HMI baseline.

## Safety boundary

The UX represents:

`MISSION → VALIDATION → READINESS → SAFETY GATE → OPERATOR APPROVAL → EXECUTION`

Planning, optimization, AI, simulation and HMI do not bypass safety authority.

## Output

Current visual reference:

`BlueSky_PRO_HMI_Total_Reconciled_1920x1080.svg`

The SVG is an editable vector reference for Figma import. The controlled logo remains a separate asset slot and is not redrawn.
