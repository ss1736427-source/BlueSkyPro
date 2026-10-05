# IF-PARAMETER — BlueSky Autopilot Parameter Interface

**Status:** STRUCTURAL DRAFT  
**Purpose:** Define safe synchronization of flight-controller parameters between BlueSky and an external autopilot.

## 1. Flow

```text
Vehicle Profile
      |
      v
BlueSky Configuration Baseline
      |
      +---- READ / COMPARE ----> Autopilot
      |                              |
      |<--------- Parameters --------+
      |
      +---- VALIDATED WRITE --------> Autopilot
      |
      +<--- ACK / RESULT / STATE ----+
```

## 2. Required operations

- identify parameter set and autopilot version;
- read parameters;
- write parameters;
- compare onboard configuration with approved baseline;
- detect changed/unknown/missing parameters;
- validate parameter dependencies;
- preserve previous configuration;
- record every controlled change;
- verify write result;
- restore an approved baseline where permitted.

## 3. Safety

BlueSky MUST distinguish between mission-level configuration and safety-critical flight-controller parameters. Automatic writes MUST be constrained by an approved configuration policy and vehicle compatibility.

## 4. Interoperability

MAVLink Parameter Protocol is the initial interoperability target. The adapter MUST not assume a fixed parameter list because supported parameters vary by firmware, vehicle type and configuration.

## 5. Open items

- parameter schema/versioning;
- compatibility matrix for ArduPilot/PX4;
- writable vs read-only policy;
- calibration parameter handling;
- transactional update/rollback strategy;
- signing/authentication requirements;
- configuration audit record format.
