# BlueSky PRO — Mission Origin, Site Anchoring and Offline Execution Baseline

**Document ID:** BS-ARCH-MISSION-ANCHOR-001  
**Status:** Architecture baseline — requirements captured; implementation and verification pending  
**Priority:** P0  
**Applies to:** All mission templates, mission planning, route editing, offline mission packages, field deployment and flight-release checks

## 1. Purpose

Define a consistent workflow that allows an operator to prepare and validate a mission at a computer before travelling to the work site, transfer it to a tablet, and use it at the site without requiring an internet connection.

The system must distinguish:
- mission origin / planned take-off point;
- first route waypoint;
- home / recovery point;
- work-area reference point;
- local coordinate-frame origin, where applicable;
- actual field position measured or confirmed at deployment.

These points may coincide in a simple mission, but they are distinct domain concepts and must not be implicitly conflated.

## 2. Core principle

**Prepare the mission independently of field connectivity; bind it to the real site explicitly; release it only after field validation.**

The mission model is canonical and vendor-independent. Vehicle-specific executable files are generated from a selected, validated mission version. See `BLUESKY_MISSION_MODEL.md`.

## 3. Two supported planning modes

### 3.1 Georeferenced planning

The operator selects a known object/site on the map or specifies a planned start location. The mission geometry is stored in a declared geographic reference system, normally WGS-84 for geographic coordinates, with explicit vertical reference for heights.

At the field site, the system compares actual position with the planned origin and checks whether the plan is being used at the intended location. A mismatch must not silently translate or rotate the mission.

### 3.2 Local / site-to-be-confirmed planning

The operator builds the task geometry in a local frame when the actual site coordinates are not yet known or when the task must be aligned in the field. The package must record:
- local frame identifier and origin definition;
- axis orientation / heading convention;
- units and handedness;
- vertical datum and height interpretation;
- the transformation method required to bind the local plan to field coordinates.

At the field site, the operator must establish the transformation using sufficient trustworthy references. A single point establishes translation only; it does not determine horizontal orientation or scale. If the local plan requires rotation, the workflow must obtain orientation from a validated heading source or additional control/reference points. The UI must disclose which parameters are measured, inferred or still unresolved.

## 4. Operator workflow

### Planning workstation

1. Start a new mission and choose a planning mode.
2. Identify or define the planned mission origin / take-off point. If the real location is not yet known, explicitly mark the origin as local / site-to-be-confirmed rather than inventing geographic coordinates.
3. Choose a mission template.
4. Generate the template's initial geometry and editable control points.
5. Define the work area, mandatory inspection points, route objective, vehicle and payload assumptions.
6. Generate route candidates and run available validations.
7. Save a versioned offline mission package with its validation report and data provenance.

### Transfer to tablet

8. Transfer the package using a supported file mechanism.
9. Verify package integrity, schema version, mission ID/version and compatibility with the target BlueSky application and vehicle adapter.
10. Open the package without internet access. Missing optional map tiles must not make the mission geometry unreadable.

### Field anchoring

11. Establish the actual field position using the configured trusted positioning method or a surveyed/otherwise verified reference.
12. Select the field anchoring workflow appropriate to the package's planning mode.
13. For a georeferenced mission, compare actual position with planned origin and enforce configured tolerance.
14. For a local mission, establish the required translation and orientation (and any other supported transform parameters) from adequate references.
15. Show the planned origin, actual position, offset, orientation status, positioning quality and unresolved inputs before accepting the binding.
16. If the transformation is ambiguous, positioning quality is insufficient, or the mismatch exceeds configured limits, block release and explain why. Do not silently relocate the mission.

### Flight release

17. Revalidate the transformed route and work envelope against terrain/obstacles, vertical reference, vehicle and payload limits, energy/recovery requirements and available regulatory/operational data.
18. Show the freshness and provenance of any cached NOTAM, weather, terrain, map or obstacle data. No internet must never be represented as proof that data is current.
19. Compile the selected mission version into a supported vehicle-specific package.
20. Where supported, upload and read back the mission, then verify semantic equivalence before flight release.

## 5. Canonical data contract

The canonical mission must represent, at minimum:

- `mission_id`, `mission_version`;
- `planning_mode`: georeferenced or local/site-to-be-confirmed;
- `coordinate_reference_system` and coordinate units;
- `vertical_reference` and vertical units;
- `planned_origin` with explicit coordinate-frame semantics;
- `first_route_waypoint` (separate field/entity);
- `home_or_recovery_point` (separate field/entity);
- `work_area_reference`;
- `local_frame_definition`, when used;
- `field_anchor` and its source, timestamp, quality and uncertainty, when bound in the field;
- `site_transform`, including method, parameters, references and residual/error estimate, when applicable;
- geometry and route versions;
- vehicle, autopilot, payload and adapter compatibility requirements;
- validation results, blockers, warnings and data snapshots;
- package schema version, integrity hash and creation metadata.

This list is a domain contract, not a claim that all fields already exist in code. The implementation must map the contract to the existing canonical model and avoid parallel competing representations.

## 6. Offline package requirements

The package must include the data required to interpret and execute the planned mission without internet:
- canonical mission and versioned geometry/routes;
- declared coordinate and vertical reference systems;
- relevant vehicle/payload configuration and compatibility metadata;
- route-generation parameters and validation evidence;
- required local terrain/obstacle/map datasets where licensing and package size permit;
- cached regulatory/weather inputs used during planning, each with source, timestamp, validity and quality;
- explicit declaration of data that is absent, expired or requires field confirmation;
- integrity and schema-version metadata.

External connectivity is not required to open the package or inspect its geometry. Connectivity-dependent data must be clearly marked as cached, unavailable or stale; the app must not imply live verification while offline.

## 7. Safety and coordinate transformation rules

- Never infer the real geographic location from a local origin alone.
- Never treat an arbitrary map click as a trusted GNSS fix.
- Never silently translate, rotate, scale or otherwise transform a mission.
- A single anchor point provides positional translation only; orientation needs a separate valid source or sufficient additional references.
- Preserve coordinate-frame and vertical-datum metadata through every transformation and export.
- Any material transformation creates a new mission version and invalidates affected route/validation results until recalculated.
- A field anchor confirms alignment, not airspace authorization or obstacle clearance.
- A take-off point does not automatically equal the first waypoint, home point or work-area centre.
- Reject unsupported coordinate frames or transformations rather than exporting ambiguous vehicle instructions.

## 8. Integration with mission templates

All mission templates must use the same origin/anchoring contract. Each template declares:
- whether its initial geometry is geographic or local;
- the initial point/line/polygon/volume configuration;
- which point is the mission origin and whether it must lie inside or near the work area;
- required orientation references, if any;
- which fields may be changed at field anchoring;
- which geometry and route calculations must be regenerated after anchoring;
- the mandatory release checks.

Template-specific control points and route algorithms remain task-specific. They must not each implement a separate, incompatible coordinate-binding workflow.

## 9. UI state model

The UI must distinguish these states:
- **Draft — origin not defined**
- **Origin defined — geometry not generated**
- **Geometry defined — route not generated**
- **Route generated — validation pending**
- **Prepared — offline package created**
- **Field anchoring required**
- **Anchored — validation required**
- **Blocked — unresolved coordinate / safety issue**
- **Validated for release** (only when all required checks pass)
- **Released / uploaded** (only after the applicable vehicle-specific checks)

The map, route table, mission profile, validator and exported package must reference the same canonical mission version and coordinate transformation.

## 10. Verification and acceptance criteria

1. A georeferenced mission can be prepared, exported, opened on a tablet offline and inspected without changing its coordinates.
2. A mission created in a local frame cannot be released until all required field-binding parameters are resolved.
3. With a local plan, a single anchor point does not falsely mark orientation as resolved.
4. A georeferenced plan started at a different field location is flagged; no silent auto-shift occurs.
5. Changing anchor, orientation, coordinate frame or vertical reference creates a new mission version and invalidates dependent route/validation evidence.
6. Planned origin, first waypoint, home/recovery point and work-area reference can be distinct and are displayed correctly.
7. Missing or stale cached environmental data is clearly identified while offline.
8. Package integrity/schema/vehicle compatibility failures block import or release with actionable errors.
9. After anchoring, the transformed route and volumetric envelope remain consistent across map, route table and mission profile.
10. Virtual Flight tests cover georeferenced planning, local-frame anchoring, missing orientation, excessive anchor offset, stale data, package corruption and successful offline import.
11. Vehicle-specific export/import and read-back are tested for each supported adapter; unsupported semantics are rejected, not silently approximated.

## 11. Implementation status

This document records architecture requirements. It does not assert that the workflow, offline package, coordinate transformation or field release gate has already been implemented. Implementation must start by tracing the current canonical mission model, route geometry, export/import formats and HMI origin handling, then add tests before enabling flight-release behavior.
