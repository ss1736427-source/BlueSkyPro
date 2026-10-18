# CHANGELOG

## 2026-08-31

- Established `PROJECT_RULES.md` as the active project rules document.
- Completed known-damage cleanup and confirmed zero known-damage findings.
- Created the source-preservation and knowledge-base structure.
- Preserved all 98 uploaded source files separately from the navigation layer.

## 2026-10-18

- Integrated the live altitude telemetry runtime/HMI bridge in `48d41c823d83cba1db9d27903fe098b120145f8e`; all 13 associated workflows passed.
- Bound `MainContent.planningResult` to the validated `PlanningBridge.result` in `150eaf91f6228c566c2370fd29355f52fe398efd`; all 13 associated workflows passed.
- Reviewed PR #52: its proposed protocol section conflicts with the existing canonical-main/no-new-branches procedure and was not adopted as written.
- Reviewed PR #53: corrected the external orchestrator's default repository and documented its limited CI scope; the stale project-state snapshot was not copied wholesale.
- Corrected Windows verification-package documentation to reflect MapTiler as the default map and the retained legacy fallback.
- The subsequent orchestrator/documentation reconciliation commit remains UNVERIFIED until checks for its resulting SHA are evaluated.
