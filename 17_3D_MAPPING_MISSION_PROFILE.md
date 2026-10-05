# BlueSky PRO — 3D Mapping Mission Profile

**Status:** ARCHITECTURE BASELINE — initial mission profile  
**Scope:** operator inputs and planning requirements for 3D mapping  
**Relationship:** extends the existing Common Mission Model; does not create a separate planner.

## 1. Purpose

The 3D Mapping profile converts the operator's survey objective and sensor requirements into machine-readable planning constraints and data-quality requirements.

It does not prescribe one photogrammetry engine.

## 2. Operator input

### Area and result
- area of interest;
- target deliverables;
- required coordinate reference system;
- required ground/sample resolution;
- required 3D reconstruction quality;
- optional control/check points.

### UAV and payload
- UAV;
- camera or LiDAR payload;
- payload configuration;
- GNSS/RTK/PPK capability;
- available battery/energy state.

### Acquisition parameters
- target altitude or resolution constraint;
- flight-line direction;
- forward overlap requirement;
- side overlap requirement;
- camera/sensor triggering mode;
- capture interval or distance where applicable;
- required viewing geometry;
- terrain-following requirement.

All numerical acquisition values are mission parameters and shall be configurable; the profile does not hard-code one universal value.

## 3. Planning transformation

Operator input becomes:

3D MAPPING REQUIREMENTS
↓
CONSTRAINED OPEN SPACE
↓
COVERAGE / ACQUISITION GEOMETRY
↓
UAV ↔ ZONE ASSIGNMENT
↓
ROUTE-IN-ZONE
↓
WIND + PERFORMANCE
↓
4D TRAJECTORY
↓
DATA ACQUISITION PLAN
↓
FINAL CHECK

For multi-UAV mapping, spatial partitioning remains the primary separation mechanism defined by the existing planning contracts.

## 4. Acquisition plan

The plan shall explicitly contain, where applicable:
- expected acquisition footprint;
- planned flight lines;
- altitude profile;
- sensor trigger plan;
- expected image/frame sequence;
- expected coverage;
- expected overlap;
- expected flight duration;
- expected energy;
- required reserve;
- launch/recovery segments.

## 5. Operator workflow

PREPARE
→ DEFINE AREA
→ SELECT UAV/PAYLOAD
→ SET 3D REQUIREMENTS
→ GENERATE PLAN
→ REVIEW COVERAGE/ALTITUDE/SENSOR
→ VALIDATE
→ EXECUTE
→ MONITOR DATA ACQUISITION
→ VERIFY DATA
→ PROCESS
→ REVIEW PRODUCT
→ RE-SURVEY IF REQUIRED

The operator remains responsible for operational authorization and human decisions. BlueSky provides planning, verification and decision support.

## 6. In-flight monitoring

The operator view should expose:
- actual vs planned route;
- coverage progress;
- UAV position/altitude;
- battery/energy;
- wind;
- communication/navigation status;
- acquisition status;
- detected data gaps or warnings.

The operator should not need to manually manage every camera trigger when the payload supports deterministic automated acquisition.

## 7. Post-flight verification

Minimum gates:

### Integrity
Every received source object matches its recorded checksum.

### Completeness
Expected acquisition sequence, telemetry and geographic coverage are accounted for.

### Acquisition quality
The dataset satisfies configured sensor and geometry requirements.

### Reconstruction quality
The generated product satisfies the configured quality criteria.

Failure produces an explicit status and reason.

## 8. Corrective mission

If a local area or acquisition segment fails verification, BlueSky should generate a corrective mission for the affected area rather than require a complete mission rebuild.

A corrective mission references:
- parent mission;
- failed verification evidence;
- affected geometry;
- required acquisition parameters;
- new planning artifact versions.

## 9. Outputs

Typical products:
- point cloud;
- orthomosaic;
- DSM;
- DTM;
- 3D mesh;
- GIS layers.

Each output references its source data, processing version and verification result.

## 10. Architectural rule

3D Mapping is a mission profile, not a parallel planning core.

It supplies requirements to the existing:
Mission Model → Planning Core → Performance → 4D Trajectory → Verification

and consumes the Mission Data Lifecycle contract after execution.

## 11. Acceptance baseline

A first executable implementation shall demonstrate:

operator-defined AOI
→ UAV/payload selection
→ configurable acquisition requirements
→ generated coverage route
→ planned acquisition manifest
→ flight execution record
→ integrity/completeness verification
→ processed product reference
→ product quality result
→ exportable Mission Data Package.

## Status

**3D MAPPING MISSION PROFILE: ARCHITECTURE BASELINE — initial version**
