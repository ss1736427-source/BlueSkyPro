# Right Panel Layout — Intermediate Checkpoint

## Status

**INTERMEDIATE / NOT VERIFIED — preserve as the current working checkpoint.**

This checkpoint records the current right-panel layout implementation for continuation. It is not a release baseline and does not claim that all reported layout defects are resolved.

## Repository state

- Repository: `ss1736427-source/BlueSkyPro`
- Branch: `fix/hmi-map-tile-loading-2026-10-09`
- Implementation commit at checkpoint: `fd2e55868ca02eb23521661d19394c5ab38810b3`
- Main implementation file: `qt/BlueSkyPRO-HMI/qml/RightPanel.ui.qml`
- Workspace geometry reference: `qt/BlueSkyPRO-HMI/qml/MainContent.ui.qml`
- Commit link: https://github.com/ss1736427-source/BlueSkyPro/commit/fd2e55868ca02eb23521661d19394c5ab38810b3

## Current implementation intent

1. ATC should remain bottom-anchored when it has been placed at the bottom and the operator has not manually moved it elsewhere.
2. The right panel is already bounded by `workspace`, which ends at `BottomToolbar.top`; panel layout calculations must not subtract the toolbar height a second time.
3. Visible panels must not overlap and must retain a 4 px gap.
4. When content grows, adjacent panels should move to avoid overlap.
5. When content shrinks, temporarily displaced panels should return to their preferred positions when those positions are free.
6. Locking positions should prevent manual dragging, not automatic reflow.

## Known verification status

The user’s latest screenshot showed ATC partly obscured by the bottom toolbar and excessive spacing between cards. The latest code change corrects the bottom-bound calculation and revises the reflow/restore pass, but the user has not yet confirmed a build and runtime test of this exact commit.

- Local MSVC build: **NOT CONFIRMED for this commit**
- Runtime verification: **PENDING**
- Startup ATC position: **PENDING**
- Dragging ATC through Alerting and moving the displaced card: **PENDING**
- Content growth/shrink and restoration of prior positions: **PENDING**
- CI: **NOT VERIFIED**; no workflow run was returned for the commit.

## Next deterministic step

Pull the branch with fast-forward only, build the existing `build` directory, then test the four behaviors above. Do not merge PR #25 or reconfigure/delete the existing build as part of this checkpoint. If a test fails, record the exact observed behavior and make the smallest targeted correction.

## Checkpoint rule

Keep this record as the latest intermediate checkpoint until the local build and runtime checks are confirmed. Do not mark the defect closed or treat this state as release-ready before verification.
