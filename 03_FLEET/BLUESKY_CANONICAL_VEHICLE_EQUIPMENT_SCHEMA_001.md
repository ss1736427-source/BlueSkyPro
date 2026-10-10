---
id: BLUESKY-CANONICAL-VEHICLE-EQUIPMENT-SCHEMA-001
type: canonical_domain_schema
status: controlled_working_draft
system: BlueSky PRO
parent: BLUESKY-UNIVERSAL-ADAPTER-CONTRACT-001
---

# BlueSky PRO — Canonical Vehicle / Equipment Schema 001

## 1. Purpose

Freeze the vendor-neutral data contract used between adapters and BlueSky core services.

The schema is implementation-oriented but remains technology-neutral. JSON, database or language-specific representations may be derived from it later without changing the canonical semantics.

## 2. Identity invariants

Every object has:

```text
id
version
status
created_at
updated_at
```

External identifiers are attributes, never replacements for canonical BlueSky IDs.

## 3. Vehicle schema

```text
Vehicle {
  vehicle_id: StableId
  fleet_id: StableId
  vehicle_profile_id: StableId
  manufacturer: String
  model: String
  serial_number: ExternalId?
  registration_id: ExternalId?
  vehicle_type: Enum
  fcs: {
    id: StableId?
    vendor: String?
    model: String?
    firmware_version: String?
  }
  configuration_baseline_id: StableId?
  capability_set: Capability[]
  readiness_state: Enum
  c2_state: Enum
  operational_state: Enum
  equipment_ids: StableId[]
  verification_state: Enum
  maintenance_state: Enum
  version: Version
}
```

## 4. Equipment schema

```text
Equipment {
  equipment_id: StableId
  vehicle_id: StableId?
  equipment_profile_id: StableId
  manufacturer: String
  model: String
  serial_number: ExternalId?
  equipment_class: Enum
  interface_id: StableId?
  capability_set: Capability[]
  configuration_version: Version?
  calibration_state: Enum
  power_state: Enum
  telemetry_state: Enum
  command_capabilities: Capability[]
  data_outputs: DataOutput[]
  compatibility_state: Enum
  verification_state: Enum
  version: Version
}
```

## 5. Capability schema

```text
Capability {
  capability_id: StableId
  supported: Boolean
  verified: Boolean
  availability_state: Enum
  constraints: Constraint[]
  source: SourceRef
  version: Version
}
```

Canonical capability state must distinguish:

```text
SUPPORTED
NOT_SUPPORTED
VERIFIED
NOT_VERIFIED
DEGRADED
```

## 6. Data output schema

```text
DataOutput {
  output_id: StableId
  type: Enum
  format: String?
  interface_id: StableId?
  recording: Boolean
  timestamped: Boolean
  validity_supported: Boolean
}
```

The schema intentionally does not prescribe a camera, sensor, file format or vendor-specific implementation.

## 7. State enums

### Readiness

```text
UNKNOWN
NOT_READY
READY
DEGRADED
```

### C2

```text
UNKNOWN
NORMAL
DEGRADED
LOST
RESTORED_PENDING_VALIDATION
```

### Compatibility

```text
UNKNOWN
COMPATIBLE
NOT_COMPATIBLE
NEEDS_REVIEW
```

### Verification

```text
NOT_VERIFIED
VERIFIED
EXPIRED
INVALIDATED
```

### Maintenance

```text
UNKNOWN
SERVICEABLE
DUE
OVERDUE
OUT_OF_SERVICE
```

## 8. Null/TBD rule

Unknown information shall be represented explicitly as absent/unknown/TBD according to the implementation representation.

The adapter shall not fabricate values to satisfy schema completeness.

## 9. Version semantics

`version` identifies the canonical object revision.

A material change to any of the following requires a new configuration-controlled version:

```text
FCS / firmware
parameters
navigation configuration
sensors
equipment
C2
safety configuration
BlueSky compatibility
```

Historical versions remain addressable for traceability.

## 10. Adapter normalization rule

```text
External Object
→ validate
→ normalize
→ canonical schema
→ persist/version
→ capability/readiness/compatibility services
```

Transport success is not equivalent to semantic validity.

## 11. Equipment terminology rule

The canonical schema contains `Equipment` and `EquipmentProfile`.

No `Payload` object exists in the BlueSky domain schema. External terminology may be preserved only as source/protocol metadata when necessary for interoperability.

## 12. Safety rule

Schema state does not grant execution authority.

In particular:

```text
READY ≠ AUTHORIZED
VERIFIED ≠ READY
SUPPORTED ≠ VERIFIED
COMPATIBLE ≠ AUTHORIZED
```

## 13. Traceability metadata

Where a field affects certification, safety, compatibility or operational decision-making, the implementation shall be able to associate it with:

```text
source
configuration baseline
adapter version
requirement
verification case
evidence
```

The exact persistence mechanism is implementation work and is not prescribed by this schema.

## 14. Conformance gate

An adapter conforms to this schema when it can:

1. create/identify the canonical object;
2. normalize supported data without semantic loss;
3. represent unsupported/invalid/stale information explicitly;
4. preserve version and source metadata;
5. expose capabilities using canonical identifiers;
6. maintain the readiness/C2/verification distinctions;
7. preserve traceability metadata required by the controlled configuration.

## 15. Current work-package transition

The canonical schema is established. Its dependent conformance matrix, contract-test stub, configuration baseline and implementation checklist are present in the repository.

The next deterministic action is the repository-level consistency validation of this dependency chain, followed by resolution of the next unresolved integration gap.

```text
CANONICAL SCHEMA
→ CONFORMANCE MATRIX
→ CONTRACT TEST STUB
→ CONFIGURATION BASELINE
→ IMPLEMENTATION CHECKLIST
→ REPOSITORY CONSISTENCY CHECK
→ NEXT UNRESOLVED INTEGRATION GAP
```

No real vehicle or Equipment test is implied by this schema.

**Status: CONTROLLED WORKING DRAFT — CANONICAL DOMAIN SCHEMA DEFINED; DEPENDENCY CHAIN READY FOR CONSISTENCY VALIDATION.**
