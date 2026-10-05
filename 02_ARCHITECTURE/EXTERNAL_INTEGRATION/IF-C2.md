# IF-C2 — Command & Control Interface

## Purpose
Define the boundary between BlueSky and the communications/C2 layer used to exchange commands, acknowledgements, telemetry and link state with UAVs.

## Scope
- command transport
- link establishment and loss detection
- addressing of multiple UAVs
- command priority and sequencing
- acknowledgement and timeout handling
- link quality/state
- degraded/offline operation
- security hooks

## Required flow
BlueSky → C2 Adapter → Transport/Radio/IP → Vehicle Adapter → Autopilot
Autopilot → Vehicle Adapter → Transport → C2 Adapter → BlueSky

The Vehicle Adapter is the controlled normalization boundary defined by the BlueSky Universal Adapter Contract. C2 transport data shall not introduce vendor-specific Vehicle/Equipment objects into BlueSky core services.

## Mandatory states
CONNECTED, DEGRADED, LOST, RECOVERING, AUTHENTICATION_FAILED.

## Key requirement
C2 transport must remain replaceable; BlueSky operational logic must not depend directly on a particular radio, modem or network technology.

## Adapter boundary

```text
External C2 / Transport
          ↓
       C2 Adapter
          ↓
Canonical C2 state/data
          ↓
Universal Vehicle / Equipment Adapter
          ↓
Canonical Vehicle / Equipment Schema
          ↓
BlueSky core services
```

The canonical Vehicle / Equipment integration is defined by:

```text
BLUESKY-UNIVERSAL-ADAPTER-CONTRACT-001
BLUESKY-CANONICAL-VEHICLE-EQUIPMENT-SCHEMA-001
```

Implementation conformance is controlled by:

```text
BLUESKY-ADAPTER-CONFORMANCE-MATRIX-001
BLUESKY-ADAPTER-CONTRACT-TEST-STUB-001
BLUESKY-ADAPTER-CONFIGURATION-BASELINE-001
BLUESKY-ADAPTER-CONFORMANCE-CHECKLIST-001
```

## Regulatory allocation

The interface is the controlled BlueSky boundary for the C2 obligations mapped from Приказ Минтранса России от 24.04.2025 №142.

| Interface area | Clause mapping | BlueSky responsibility | Status |
|---|---|---|---|
| C2 mode / applicability | C2-142-001, C2-142-006 | expose configured C2 mode to dependent functions | WORKING |
| C2 state / degradation | C2-142-018, C2-142-020 | expose normalized state and degradation/loss indication | WORKING |
| C2 quality parameters | C2-142-004, C2-142-011, C2-142-013, C2-142-014, C2-142-019, C2-142-025 | consume controlled parameters; values remain TBD until approved basis | WORKING / TBD |
| Provider-C2 boundary | C2-142-003, C2-142-008, C2-142-010, C2-142-012 | validate and monitor only the defined provider interface; provider infrastructure remains external | WORKING |
| Event / state recording | C2-142-021, C2-142-024 | provide C2 events, timestamps and diagnostics to controlled journal/data layer | WORKING |
| Loss response | C2-142-023, C2-142-024 | pass C2 condition into approved safety/flight response chain | WORKING |
| Time reference | C2-142-005, C2-142-021 | preserve controlled ordering/timing metadata for C2 data and events | WORKING |
| Operator information | C2-142-015, C2-142-020 | expose required C2 status/warnings through controlled HMI interface | WORKING |

## Responsibility boundary

```text
BlueSky
  ├─ C2 mode/configuration
  ├─ link-state monitoring
  ├─ interface-side QoS/status handling
  ├─ event recording
  ├─ safety/flight workflow input
  └─ operator information

External C2 Provider
  ├─ provider infrastructure
  ├─ provider-side service quality
  ├─ provider-side security
  └─ provider maintenance/failure records
```

No provider-side responsibility is converted into a BlueSky system requirement by this interface document.

## Traceability

```text
REG-013
→ C2_CLAUSE_LEVEL_MAPPING_001
→ C2_SYSTEM_REQUIREMENTS_BASELINE_001
→ C2_REQUIREMENTS_RECONCILIATION_001
→ IF-C2
→ BLUESKY-UNIVERSAL-ADAPTER-CONTRACT-001
→ BLUESKY-CANONICAL-VEHICLE-EQUIPMENT-SCHEMA-001
→ BLUESKY-ADAPTER-CONFORMANCE-MATRIX-001
→ BLUESKY-ADAPTER-CONTRACT-TEST-STUB-001
→ BLUESKY-ADAPTER-CONFIGURATION-BASELINE-001
→ BLUESKY-ADAPTER-CONFORMANCE-CHECKLIST-001
→ C2 verification cases
→ evidence
```

Quantitative acceptance thresholds remain `TBD`. No compliance, baseline or verification claim is made by this interface document.
