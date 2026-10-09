---
id: MT01-MT02-CASE-BINDING-001
type: verification_case_binding
status: draft_for_agreement
authority: MASTER-REQUIREMENTS-REGISTER-001; VERIFICATION-REGISTER-001; BLUESKY_ALGORITHM_ORCHESTRATION_AND_OPTIMIZATION
---

# MT-01 / MT-02 Case-Level Requirement and Configuration Binding

## 1. Purpose

This record binds the first baseline verification cases to the currently controlled requirement/design allocation, dataset identity and execution prerequisites.

It deliberately distinguishes:
- requirement identity/allocation;
- algorithm/design acceptance basis;
- dataset identity;
- controlled execution parameters;
- actual evidence.

No case is marked executed or passed.

## 2. V-M01-01 — MT-01 positive baseline

### Case identity
- Verification: V-M01-01
- Dataset: MT01-T01
- Method: TEST / SIMULATION
- Objective: positive baseline mapping of a valid AOI.

### Requirement/design basis

| Layer | Controlled basis | Relationship |
|---|---|---|
| Requirement identity | SYS-REQ-008 | DIRECT DEPENDENCY — readiness input |
| Requirement identity | SYS-REQ-035 | DIRECT DEPENDENCY — task/capability mapping |
| Requirement identity | SYS-REQ-076 | DIRECT DEPENDENCY — UAV capability profile |
| Algorithm | MT-01 Sections 22, 24, 26–29, 40 | PRIMARY ALGORITHM BASIS |
| Architecture | Mission → Validation → Readiness → Safety Gate → Approval → Execution | EXECUTION AUTHORITY BOUNDARY |
| Dataset | MT01-T01 | CONTROLLED DATASET ID |

The Master Requirements Register establishes the identities and controlled allocation relationships. It does not provide the complete controlled wording of each requirement in this case record; therefore no wording is reproduced or inferred here.

### Algorithm acceptance basis

The case shall demonstrate:
1. input integrity succeeds;
2. constrained open space is produced;
3. acquisition geometry is feasible;
4. coverage decomposition/tracks are generated;
5. candidate route is spatially feasible;
6. performance and energy calculations are valid;
7. trajectory is valid;
8. acquisition events satisfy the applicable quality model;
9. mapping quality gate is satisfied;
10. final integrity validation succeeds;
11. selected plan contains required provenance/version identity.

Mandatory quality components are evaluated independently. No aggregate score may hide a failed mandatory component.

### Quantitative acceptance criteria

The case does not invent universal values.

Actual thresholds shall be taken from the active mission requirement / quality profile, UAV configuration, equipment profile, algorithm configuration, safety policy and controlled numerical parameter registry.

If a mandatory parameter is not available from an authoritative or approved source, the case remains OPEN and is not executed with a fabricated value.

### Required execution configuration

Before execution, bind:
- mission revision;
- AOI geometry and CRS;
- terrain/elevation source and version;
- obstacle/restriction state;
- authorization state;
- weather/wind snapshot;
- UAV configuration/version;
- equipment configuration/calibration/version;
- battery/SOC/SOH/degradation state;
- C2 state;
- objective profile;
- algorithm/planner version;
- numerical parameter configuration;
- execution environment.

### Expected result

A valid candidate plan is produced and selected, or the case is explicitly blocked with a controlled failure class. For the positive baseline dataset, a blocking outcome is a test failure unless the dataset/configuration itself is found invalid and that invalidity is controlled as a dataset anomaly.

No result is currently recorded.

## 3. V-M02-01 — MT-02 positive baseline

### Case identity
- Verification: V-M02-01
- Dataset: MT02-T21
- Method: TEST / SIMULATION
- Objective: positive baseline 3D reconstruction.

### Requirement/design basis

| Layer | Controlled basis | Relationship |
|---|---|---|
| Requirement identity | SYS-REQ-008 | DIRECT DEPENDENCY — readiness input |
| Requirement identity | SYS-REQ-035 | DIRECT DEPENDENCY — task/capability mapping |
| Requirement identity | SYS-REQ-076 | DIRECT DEPENDENCY — UAV capability profile |
| Algorithm | MT-02 Sections 23, 25, 26–29, 41 | PRIMARY ALGORITHM BASIS |
| Architecture | Mission → Validation → Readiness → Safety Gate → Approval → Execution | EXECUTION AUTHORITY BOUNDARY |
| Dataset | MT02-T21 | CONTROLLED DATASET ID |

The complete controlled wording of the cited SYS requirements is not reproduced because the current Master Requirements Register provides identity/allocation, while the authoritative wording remains subject to exact requirement-content reconciliation.

### Algorithm acceptance basis

The case shall demonstrate:
1. target representation is valid;
2. observation domain is constructed;
3. target elements are discretized;
4. feasible viewpoints are generated;
5. viewpoint feasibility filtering succeeds;
6. visibility is evaluated;
7. required target observations are selected;
8. viewpoint transitions are feasible;
9. trajectory is valid;
10. acquisition events satisfy observation geometry;
11. reconstruction quality gates are satisfied;
12. final integrity validation succeeds;
13. selected plan contains required provenance/version identity.

The MT-02 quality model evaluates target coverage, observation count, incidence/distance, parallax, visibility, observation-network connectivity and sensor-specific quality as applicable. A mandatory failed component cannot be hidden by an aggregate score.

### Quantitative acceptance criteria

No universal numerical threshold is introduced.

Values shall come from the active mission/quality profile, target model, UAV/equipment configuration, algorithm configuration and approved numerical parameter registry.

Missing mandatory values remain OPEN / BLOCKED_INPUT according to the existing algorithm rules.

### Required execution configuration

Before execution, bind:
- mission revision;
- target representation and version;
- target geometry/mesh/point-cloud/volume data;
- CRS/reference frame;
- terrain/elevation source;
- obstacle/restriction state;
- authorization state;
- weather/wind snapshot;
- UAV configuration/version;
- camera/gimbal/sensor configuration and calibration;
- battery/SOC/SOH/degradation;
- C2 state;
- objective profile;
- algorithm/planner version;
- numerical parameter configuration;
- execution environment.

### Expected result

A valid reconstruction acquisition candidate is produced and selected, satisfying the applicable target-observation and quality gates, or the case is explicitly rejected with a controlled failure class.

No result is currently recorded.

## 4. Common evidence requirements

Execution evidence must bind:

Verification ID
→ Requirement/design basis
→ Dataset ID + version
→ Mission/target revision
→ Planner/algorithm version
→ Parameter configuration
→ UAV/equipment configuration
→ Environment snapshot
→ Execution environment
→ Expected result
→ Actual result
→ Pass/Fail
→ Anomalies
→ Evidence ID
→ Reviewer

An unbound simulation output is not controlled evidence.

## 5. Current binding state

| Item | V-M01-01 | V-M02-01 |
|---|---|---|
| Verification identity | CONTROLLED | CONTROLLED |
| Dataset identity | CONTROLLED | CONTROLLED |
| Requirement identity/allocation | CONTROLLED / PARTIAL | CONTROLLED / PARTIAL |
| Algorithm acceptance basis | CONTROLLED | CONTROLLED |
| Exact requirement wording | OPEN CONTENT RECONCILIATION | OPEN CONTENT RECONCILIATION |
| Numerical parameter binding | OPEN | OPEN |
| Execution configuration | OPEN | OPEN |
| Execution | NOT DONE | NOT DONE |
| Evidence | NOT DONE | NOT DONE |

## 6. Gate

The two baseline cases are now case-level defined and allocation-bound, but they are not verification-closed.

Next action is controlled parameter/configuration binding for MT01-T01 and MT02-T21, followed by execution preparation. No additional verification IDs are required at this stage.
