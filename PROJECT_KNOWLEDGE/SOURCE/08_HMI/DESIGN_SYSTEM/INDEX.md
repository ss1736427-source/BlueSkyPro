# BlueSky PRO — HMI Documentation Index

Status: WORKING REFERENCE

## Purpose

This index defines the controlled order of the HMI documentation after the 2026-09-19 panel-behavior consolidation.

## Document hierarchy

1. **Visual source of truth**
   - `BLUESKY_PRO_DESIGN_SYSTEM.md`
   - Colors, typography, visual semantics, component rules.

2. **DS component master**
   - `BLUESKY_PRO_DS_PANEL_MASTER_001.md`
   - Reusable panel anatomy, states, tokens, action/warning treatment and workspace variants.
   - Visual reference: `BLUESKY_PRO_DS_PANEL_MASTER_001.svg`.

3. **Screen architecture and panel behavior**
   - `BLUESKY_PRO_PANEL_BEHAVIOR_SPECIFICATION_001.md`
   - Six-level screen structure, panel responsibilities, states, visibility, collapse/expand, configuration, synchronization and contextual behavior.

4. **Operational screen reference**
   - `BLUESKY_PRO_SCREEN_REFERENCE.md`
   - Detailed controlled behavior of the Right Panel and local UAV decision context.

5. **Layout and ergonomics**
   - `BLUESKY_PRO_PANEL_LAYOUT.md`
   - Panel classes, map priority, layout variants and UAV-panel configuration.

6. **HMI baseline**
   - `BLUESKY_PRO_HMI_DESIGN_BASELINE.md`
   - Component model, UX priorities and general HMI principles.

7. **Total reconciliation**
   - `BLUESKY_PRO_HMI_TOTAL_RECONCILIATION_001.md`
   - Reconciled source hierarchy and explicit unresolved conflicts.

8. **UI traceability**
   - `BLUESKY_PRO_UI_TRACEABILITY.md`
   - UI-ID allocation and requirement/safety boundary.

9. **Figma / visual handoff**
   - `BLUESKY_PRO_FIGMA_HANDOFF.md`
   - Componentization and Qt 6 + QML handoff.

10. **Assets / tokens**
   - `BLUESKY_PRO_COLORS.md`
   - `BLUESKY_PRO_TYPOGRAPHY.md`
   - `BLUESKY_PRO_SOURCE_ASSET_REGISTER.md`
   - `FIGMA/BlueSky_PRO_Figma_Visual_Tokens.json`
   - `BLUESKY_PRO_LOGO.md`
   - `ASSETS/`

## 2026-09-19 controlled consolidation

The following Git history is the authoritative source for the latest panel behavior:
- `257ed2be` — responsive Top Header structure
- `0aea2f39` — Bottom Toolbar panel control behavior
- `36748919` — complete Bottom Toolbar specification
- `9fde6b9c` — controlled default UAV Panel composition
- `438cf15` — Left Panel Mission Templates structure/state
- `954c50ba` — adaptive Mission Templates behavior/tool growth
- `c2b9d1dc` — task-used Mission Templates filtering
- `baf999d2` — Left Panel mission context and Mission ID
- `65df3ea6` — Right Panel operational state
- `2640be05` — UAV decision support / RETURN context
- `c079957f` — configurable panel tool visibility

All of the above modify the controlled screen reference rather than creating parallel HMI specifications.

## Cleanup rule

Historical imports, snapshots and legacy material are retained for traceability but are not active HMI sources. Do not edit archived material to resolve current HMI behavior. New decisions belong in the controlled documents above.

## Implementation rule

The canonical order for Qt/QML implementation is:

`PANEL BEHAVIOR → SCREEN REFERENCE → DESIGN SYSTEM → DS PANEL MASTER → PANEL LAYOUT → UI TRACEABILITY → Figma/Qt handoff`

Where documents disagree, preserve the disagreement in the reconciliation record until an explicit project decision resolves it.
