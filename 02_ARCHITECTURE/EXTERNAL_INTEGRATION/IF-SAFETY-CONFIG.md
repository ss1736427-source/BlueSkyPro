# IF-SAFETY-CONFIG — BlueSky Safety Configuration Interface

**Status:** STRUCTURAL DRAFT  
**Purpose:** Keep BlueSky operational safety assumptions consistent with the actual vehicle/autopilot safety configuration.

## Flow

```text
BlueSky Safety Model
        |
        v
Vehicle-specific Safety Policy
        |
        +---- compare ----> Onboard Autopilot Safety Configuration
        |
        v
Safety Gate
        |
        v
Approved Flight Configuration
```

## Required domains

- geofence/fence;
- rally/return points;
- RTL/return behavior;
- loss-of-C2 behavior;
- loss-of-GNSS behavior;
- battery failsafe;
- link-loss failsafe;
- altitude/speed limits;
- landing/termination policy;
- navigation/position validity;
- vehicle-specific emergency constraints.

## Principle

BlueSky MUST NOT declare a vehicle operationally ready solely from its own planning model. Critical onboard safety configuration MUST be available for verification and its validity status MUST participate in the Readiness/Safety Gate.

## Open items

- authoritative parameter mapping per autopilot;
- configuration precedence;
- permitted automatic changes;
- pre-flight discrepancy handling;
- evidence and audit record.
