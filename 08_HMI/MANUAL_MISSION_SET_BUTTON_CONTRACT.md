# Manual Mission SET and Template-Aware Geometry Contract

**Status:** implementation requirement  
**Scope:** manual mission composition, map geometry, Common Mission Model, Planning Core submission  
**Canonical branch:** `main`

## 1. SET lifecycle

1. The manual mission action is labelled **SET** (УСТАНОВИТЬ), not Validate / Validation.
2. SET is available only in manual mission setup, after the operator has selected at least one compatible mission template and completed the required geometry and parameters.
3. SET submits the actual composed mission model to the authoritative Planning Core. The HMI must not calculate or locally infer validation success.
4. While the authoritative check is pending, show a checking state; do not show success.
5. SET becomes fully green only after a fresh result for the active mission reports `finalGateStatus=PASS`, `releaseStatus=RELEASE_ELIGIBLE`, and `verified=true`.
6. Failed, blocked, stale, or mismatched results must not turn SET green. Return to editable setup when the check completes unsuccessfully.
7. A UI state transition, click, old result, or result for another mission is not evidence of success.
8. SET does not by itself authorize flight.

## 2. Template-aware manual geometry

Manual geometry is not a generic freehand route independent of mission intent. It must be defined by the selected template set and its task requirements.

- Each selected template contributes its required geometry semantics, parameters, payload/sensor requirements, quality criteria, and applicable constraints.
- The map editing tools and required geometry must adapt to the selected template(s). Depending on the template, the geometry may represent an area of interest, coverage/grid, corridor, asset/inspection points, search area, point-to-point delivery route, observation/orbit points, or another explicitly supported geometry type. Do not expose a geometry type as supported until its input and validation path exists.
- The operator may combine multiple compatible templates into one manual mission. The system must compose their requirements into one Common Mission Model rather than create disconnected duplicate missions.
- Template composition must be intelligent and non-redundant: merge equivalent shared requirements once; preserve requirements that are genuinely different; detect incompatible geometry, payload, quality, altitude, timing, or operational constraints; and report unresolved conflicts instead of silently discarding or duplicating them.
- Geometry is shared across templates only when it satisfies each participating template's semantics. If a template needs a distinct geometry object or a different segment of the mission, preserve that distinction explicitly in the mission model.
- Changing the selected template set must re-evaluate geometry requirements and parameters. Geometry that no longer satisfies the selected set must be marked for correction; it must not be silently reinterpreted.
- Manual choices are explicit operator inputs. Any system-derived template suggestion or geometry interpretation must remain distinguishable from operator-confirmed choices.

## 3. Common Mission Model and Planning pipeline

Use one canonical mission model for automatic and manual missions. Manual mode changes how intent, templates, geometry, and parameters are supplied; it must not bypass the shared planning and safety pipeline.

Required logical flow:

`Selected template(s) + operator inputs`
→ `template requirement composition`
→ `Common Mission Model (deduplicated requirements + explicit geometry)`
→ `input completeness and compatibility checks`
→ `Planning Core route generation / optimization`
→ `quality and safety verification`
→ `fresh authoritative result`
→ `SET green only on release-eligible PASS`

The Common Mission Model must retain, at minimum:
- mission identity/version and selected template IDs;
- each geometry object with its type, coordinate reference system, coordinates, and template/task associations;
- task goals and quality criteria contributed by the templates;
- resolved UAV, payload/sensor and capability requirements;
- altitude reference, timing, environment, airspace and operational constraints with traceable sources;
- unresolved mandatory inputs and compatibility conflicts;
- version/lineage identifiers so a planner result can be matched to the exact submitted mission and geometry.

Store each shared fact or constraint once where its meaning is identical, with references from the applicable tasks/templates. Do not flatten semantically different requirements into one value merely to reduce fields. The model must be minimal without losing traceability or safety meaning.

## 4. Template library boundary

Use the approved 13-template catalog in `08_PLANNING/BLUESKY_MISSION_TEMPLATE_CATALOG.md`. Template names are task classifications, not hard-coded algorithms. The template set determines required capabilities and task semantics; internal algorithm selection remains the Planning Core's responsibility.

## 5. Integration and fail-closed requirements

The SET request must contain the real manual Common Mission Model and its actual geometry, not a fabricated route candidate or placeholder coordinates. The planner must receive enough information to build or validate a route under the selected templates' requirements.

Do not invent missing geometry, environment snapshots, vehicle state, payload configuration, or route candidates. If the current bridge contract cannot carry the submitted manual mission input, extend the contract and its schema/validator together before claiming end-to-end integration. A result may turn SET green only if it is fresh and traceably refers to the exact submitted mission version and geometry.

## 6. Acceptance criteria

1. Manual setup requires at least one selected, compatible template.
2. Geometry tools and required fields reflect the selected template set.
3. Two or more compatible templates can be composed into one mission model.
4. Shared requirements are deduplicated; distinct requirements remain distinct.
5. Conflicting template requirements block SET and identify the conflict.
6. Editing templates or geometry invalidates any earlier validation for the previous mission version.
7. The actual composed model is submitted to Planning Core; no HMI-only or fabricated PASS is possible.
8. SET turns fully green only on a fresh, authoritative, traceable PASS with release eligibility.
9. The same Planning Core and safety gates apply to manual and automatic missions.
