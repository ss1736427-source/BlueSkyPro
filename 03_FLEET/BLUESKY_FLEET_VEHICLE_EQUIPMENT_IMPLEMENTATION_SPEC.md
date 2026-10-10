---
id: BLUESKY-FLEET-VEHICLE-EQUIPMENT-IMPLEMENTATION-SPEC-001
type: implementation_specification
status: controlled_working_draft
system: BlueSky PRO
parent: BLUESKY_FLEET_VEHICLE_EQUIPMENT_MODEL.md
---

# BlueSky PRO — Fleet / Vehicle / Equipment Implementation Specification

## 1. Purpose

Define the smallest implementation-ready contract for Fleet, Vehicle and Equipment domain objects and their integration boundary.

This specification derives from the approved working data model and does not introduce vendor-specific mission logic.

## 2. Implementation boundary

```text
DOMAIN MODEL
    ↓
FLEET / VEHICLE / EQUIPMENT SERVICES
    ↓
PERSISTENCE + VERSIONING
    ↓
UNIVERSAL ADAPTER CONTRACT
    ↓
EXTERNAL PROTOCOL / VEHICLE / EQUIPMENT
```

The domain layer shall not depend directly on MAVLink, ArduPilot, PX4, OEM APIs, camera protocols, C2 transports or provider-specific formats.

## 3. Canonical object identifiers

Each persistent object shall have a stable BlueSky identifier and version where the object is configuration-controlled.

```text
FleetId
VehicleId
EquipmentId
VehicleProfileId
EquipmentProfileId
ConfigurationBaselineId
```

External identifiers are stored as attributes and shall not replace the BlueSky identity.

## 4. Fleet aggregate

Minimum implementation fields:

```text
fleet_id
name
organization_id
status
active_configuration_id
vehicle_ids[]
equipment_profile_ids[]
created_at
updated_at
version
```

Fleet status shall not be inferred from individual vehicle availability alone.

## 5. Vehicle aggregate

Minimum implementation fields:

```text
vehicle_id
fleet_id
manufacturer
model
serial_number
registration_id
vehicle_type
vehicle_profile_id
fcs_id
fcs_vendor
fcs_model
firmware_version
configuration_baseline_id
capability_set
readiness_state
c2_state
operational_state
equipment_ids[]
verification_state
maintenance_state
version
```

Fields that are not available for a specific vehicle remain `TBD`/absent according to the schema rule; values shall not be fabricated.

## 6. Equipment aggregate

Minimum implementation fields:

```text
equipment_id
vehicle_id
manufacturer
model
serial_number
equipment_class
equipment_profile_id
interface_id
capability_set
configuration_version
calibration_state
power_state
telemetry_state
command_capabilities
data_outputs
compatibility_state
verification_state
version
```

Equipment is a first-class domain object. It is not represented as a generic mission attachment or as a special-case vendor object.

## 7. Capability representation

Capabilities shall be represented as normalized identifiers with controlled state.

```text
CapabilityId
supported
verified
availability_state
constraints[]
source
version
```

Minimum capability state:

```text
SUPPORTED
NOT_SUPPORTED
VERIFIED
NOT_VERIFIED
DEGRADED
```

`SUPPORTED` does not imply `VERIFIED`.

## 8. Readiness and authorization separation

The implementation shall preserve the three-way separation:

```text
CAPABILITY
READINESS
AUTHORIZATION
```

A capability service answers whether a function can be performed. A readiness service answers whether the current configuration and conditions permit safe operation. Authorization is obtained from the applicable regulatory/operational authority chain.

No single `ready` flag may replace these independent states.

## 9. Configuration baseline

A vehicle configuration baseline shall bind:

```text
Vehicle
+ FCS / Firmware
+ Parameters
+ Navigation / Sensors
+ Equipment
+ C2
+ Safety configuration
+ BlueSky software compatibility
```

A baseline is immutable after release. A material change creates a new version.

## 10. Equipment compatibility

Compatibility evaluation shall be deterministic:

```text
Vehicle profile
       +
Equipment profile
       ↓
Interface compatibility
       ↓
Capability compatibility
       ↓
Mass / balance compatibility
       ↓
Power compatibility
       ↓
Operational-limit compatibility
       ↓
Verification status
       ↓
COMPATIBLE / NOT COMPATIBLE / NEEDS REVIEW
```

A compatibility result shall be traceable to the profile/configuration versions used for the decision.

## 11. Onboarding state machine

```text
DISCOVERED
  ↓
IDENTIFIED
  ↓
PROFILE_MATCHED
  ↓
CONFIGURATION_READ
  ↓
EQUIPMENT_DETECTED
  ↓
CAPABILITIES_RESOLVED
  ↓
COMPATIBILITY_CHECKED
  ↓
VERIFICATION_CHECKED
  ↓
AVAILABLE / NEEDS_ACTION
```

Failure at any state shall preserve the diagnostic reason and prevent silent promotion to an available configuration.

## 12. Persistence rules

The persistence model shall preserve:

- stable object identity;
- object version;
- source of externally supplied attributes;
- configuration baseline relationship;
- verification state;
- change history;
- effective timestamps where operationally relevant.

Historical versions shall remain addressable for Flight Record and regulatory traceability.

## 13. Adapter contract boundary

The universal adapter contract is the controlled integration boundary between the canonical BlueSky domain model and external Vehicle / Equipment representations.

```text
External Vehicle / Equipment API
            ↓
Universal Adapter Contract
            ↓
Canonical Vehicle / Equipment Schema
            ↓
Core services
```

The detailed contract is defined in:

```text
BLUESKY-UNIVERSAL-ADAPTER-CONTRACT-001
```

The canonical schema is defined in:

```text
BLUESKY-CANONICAL-VEHICLE-EQUIPMENT-SCHEMA-001
```

No adapter may alter the meaning of a canonical mission requirement to accommodate a vendor protocol. Unsupported functions shall be represented explicitly through capability state.

## 14. Adapter conformance and verification hooks

Adapter implementation shall be controlled through:

```text
BLUESKY-ADAPTER-CONFORMANCE-MATRIX-001
        ↓
BLUESKY-ADAPTER-CONTRACT-TEST-STUB-001
        ↓
BLUESKY-ADAPTER-CONFIGURATION-BASELINE-001
        ↓
BLUESKY-ADAPTER-CONFORMANCE-CHECKLIST-001
```

The controlled matrix defines conformance obligations. The contract-test stub defines deterministic mock vectors. The configuration baseline makes later execution reproducible. The checklist controls implementation readiness.

Real hardware and flight evidence are not required to define these hooks. Execution results remain deferred to the V&V stage.

## 15. Rework-control rule

Any later requirement or architecture change shall be handled through:

```text
Requirement change
→ impact assessment
→ domain-model impact
→ interface impact
→ implementation impact
→ verification impact
→ controlled update
```

The canonical Fleet/Vehicle/Equipment objects shall not be redesigned independently for UI, adapter or individual vendor integration.

## 16. Phase 3 gate

The implementation specification, universal adapter contract, canonical schema, conformance matrix, contract-test stub, configuration baseline and implementation checklist together define the current pre-hardware implementation boundary.

The next deterministic implementation step is to bind these artifacts to the existing repository interface/service layer, reusing existing interfaces where present and creating only a stub where a genuine gap exists.

No real-test status is assigned by this document.

**Status: CONTROLLED WORKING DRAFT — FLEET/VEHICLE/EQUIPMENT IMPLEMENTATION CHAIN SYNCHRONIZED.**
