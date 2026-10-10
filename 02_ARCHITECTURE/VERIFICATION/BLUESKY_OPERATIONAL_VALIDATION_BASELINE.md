# BlueSky PRO — Operational Validation Baseline

**Status:** ARCHITECTURE BASELINE — P0 verification contract

## 1. Purpose

Define the evidence chain proving that a mission solution is executable on the selected UAV configuration and remains controlled through preparation, upload and execution.

## 2. Principle

```text
PLAN
 ↓
STATIC VALIDATION
 ↓
SIL
 ↓
HIL
 ↓
REAL UAV
 ↓
FLIGHT EVIDENCE
 ↓
REPLAY / ANALYSIS
```

Not every mission requires every stage operationally, but the applicable verification path shall be defined and traceable.

## 3. Validation gates

### Gate A — Static

Verify before simulation or upload:

- mission schema;
- coordinates and reference systems;
- vehicle/payload compatibility;
- route constraints;
- airspace/regulatory constraints;
- energy reserve;
- C2 requirements;
- failsafe configuration;
- timing and coordination constraints.

### Gate B — SIL

Execute the mission against a software-in-the-loop vehicle/autopilot model where applicable. Verify mission logic, state transitions, commands, telemetry, failures and replanning without requiring the physical aircraft.

### Gate C — HIL

Verify interfaces and timing with hardware in the loop where required. The objective is to expose integration faults that pure simulation cannot reveal.

### Gate D — Real UAV

Execute controlled representative tests on the actual vehicle, autopilot, payload, C2 configuration and operational environment.

## 4. Automated pre-flight validation

The system shall perform automated checks and present the pilot with a concise readiness result.

```text
READY
NOT READY
READY WITH CONDITION
```

Every failed gate shall have a machine-readable reason and a human-readable concise explanation.

## 5. Energy validation

The same authoritative energy model used by planning shall be used by validation.

Validation shall account for the selected route, current battery state/health, payload, propulsion configuration, environmental conditions and required recovery reserve.

A mission failing the mandatory energy gate cannot be released merely because another objective is satisfied.

## 6. Autopilot validation

The selected adapter shall verify, where supported:

- vehicle identity;
- autopilot identity/version;
- required capabilities;
- required parameters/configuration;
- mission compatibility;
- command availability;
- failsafe configuration;
- upload/read-back integrity.

## 7. C2 validation

Before release, the system shall verify the required communication configuration and expected operational channel availability.

Validation shall also establish the applicable behaviour for degradation, loss and recovery of C2.

## 8. Mission package validation

The exact released Mission Package is the object being validated. Any change to executable content creates a new version and invalidates the previous release state.

## 9. SIL/HIL test scenarios

The verification library shall include, as applicable:

- nominal mission;
- waypoint/trajectory errors;
- insufficient energy;
- increasing headwind;
- degraded navigation;
- GNSS loss;
- C2 degradation;
- complete C2 loss;
- C2 recovery;
- autopilot rejection;
- mission upload interruption;
- read-back mismatch;
- stale telemetry;
- vehicle state divergence;
- replanning trigger;
- multi-UAV communication loss;
- multi-UAV vehicle failure.

## 10. Replay

Flight logs and BlueSky operational events shall be correlated by mission/package/vehicle identifiers.

Replay shall reconstruct the relevant operational timeline and allow comparison of:

- predicted route vs actual route;
- predicted energy vs actual consumption;
- predicted wind vs observed wind;
- planned state vs actual vehicle state;
- commands vs acknowledgements/results;
- C2 state vs vehicle behaviour;
- replanning decisions and their causes.

## 11. Evidence

For each released mission, the evidence chain shall identify at minimum:

- mission version;
- mission package version;
- vehicle configuration version;
- payload configuration version;
- autopilot/adapter version;
- algorithm/orchestrator version;
- relevant environmental data;
- regulatory/airspace data and validity;
- validation results;
- operator actions;
- execution logs;
- final outcome.

## 12. Release decision

```text
ALL MANDATORY GATES PASS
          ↓
      RELEASED
          ↓
   UPLOAD / READ-BACK
          ↓
       EXECUTE
```

Any failed mandatory gate blocks release.

## 13. Runtime verification

Validation does not end at takeoff. Runtime events may invalidate the current solution and trigger:

- continued execution;
- local recalculation;
- full replanning;
- recovery/failsafe;
- mission termination.

The decision shall be based on the Runtime State Machine and current safety/energy constraints.

## 14. Acceptance criteria

The operational validation contract is complete when:

1. mandatory pre-flight checks are automated;
2. validation uses the exact released package;
3. SIL/HIL paths are defined;
4. representative real-UAV verification is defined;
5. failure scenarios are executable as tests;
6. C2 and autopilot failures are covered;
7. replay can correlate prediction and actual execution;
8. evidence is retained and traceable;
9. mandatory failures block release;
10. runtime changes can invalidate and revalidate a mission.

## 15. Verification status

A capability shall not be labelled `VERIFIED` because its specification exists. Verification requires objective evidence from the applicable test level.

```text
SPECIFIED
   ↓
IMPLEMENTED
   ↓
INTEGRATED
   ↓
TESTED
   ↓
VERIFIED
```
