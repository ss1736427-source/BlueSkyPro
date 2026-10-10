---
id: BLUESKY-UNIVERSAL-ADAPTER-CONTRACT-001
type: universal_adapter_contract
status: controlled_working_draft
system: BlueSky PRO
parent: BLUESKY_FLEET_VEHICLE_EQUIPMENT_IMPLEMENTATION_SPEC-001
---

# BlueSky PRO — Universal Vehicle / Equipment Adapter Contract 001

## 1. Purpose

Define a vendor-neutral contract between external vehicle/equipment integrations and the BlueSky canonical domain model.

The contract is an integration boundary. It does not prescribe a specific transport, SDK, autopilot, equipment protocol or manufacturer implementation.

## 2. Design rule

```text
EXTERNAL PROTOCOL
      ↓
ADAPTER
      ↓
CANONICAL BLUE SKY MODEL
      ↓
CORE SERVICES
```

Core services shall depend on canonical semantics, not vendor-specific protocol objects.

## 3. Controlled dependencies

The contract is reconciled with the controlled adapter chain:

```text
BLUESKY-CANONICAL-VEHICLE-EQUIPMENT-SCHEMA-001
BLUESKY-ADAPTER-CONFORMANCE-MATRIX-001
BLUESKY-ADAPTER-CONTRACT-TEST-STUB-001
BLUESKY-ADAPTER-CONFIGURATION-BASELINE-001
BLUESKY-ADAPTER-CONFORMANCE-CHECKLIST-001
```

These references establish traceability only; they do not imply implementation conformance or verification.

## 4. Adapter identity

Every adapter implementation shall expose:

```text
adapter_id
adapter_version
vendor
protocol
supported_vehicle_profiles[]
supported_equipment_profiles[]
capabilities[]
configuration_schema_version
```

Adapter identity and version are configuration-controlled.

## 5. Vehicle adapter contract

### Required operations

```text
discover()
identify()
read_profile()
read_configuration()
read_state()
read_capabilities()
read_equipment()
read_health()
read_c2_state()
subscribe_telemetry()
translate_command()
```

An adapter shall return canonical data structures or an explicit unsupported/error state.

### Required properties

```text
Deterministic
Versioned
Traceable
Idempotent where applicable
Timeout-bounded
Failure-explicit
```

## 6. Equipment adapter contract

### Required operations

```text
identify()
read_profile()
read_configuration()
read_capabilities()
read_state()
read_telemetry()
translate_action()
read_data_outputs()
```

Equipment actions shall use canonical BlueSky semantic operations, for example:

```text
CAPTURE
START_RECORDING
STOP_RECORDING
SET_CAMERA_MODE
SET_SENSOR_MODE
SET_GIMBAL
TRIGGER_EQUIPMENT
```

The adapter translates the semantic operation into the concrete equipment protocol.

## 7. Canonical response envelope

Adapter responses shall provide enough metadata to determine whether returned information is usable:

```text
request_id
adapter_id
adapter_version
object_id
object_version
timestamp
validity
quality
source
data
error
```

`data` shall never be treated as valid merely because transport succeeded.

## 8. Failure model

The adapter shall distinguish at minimum:

```text
UNSUPPORTED
INVALID_REQUEST
TIMEOUT
NO_CONNECTION
STALE_DATA
INVALID_DATA
PROTOCOL_ERROR
DEVICE_ERROR
VERSION_MISMATCH
AUTHENTICATION_ERROR
CONFIGURATION_ERROR
```

Failures shall not be converted into normal values or silently discarded.

## 9. Capability mapping

External capabilities shall be normalized to BlueSky `CapabilityId` values.

Mapping example:

```text
External capability
        ↓
Adapter mapping
        ↓
CapabilityId
        ↓
SUPPORTED / NOT_SUPPORTED
        ↓
VERIFIED / NOT_VERIFIED / DEGRADED
```

An adapter must not report `VERIFIED` solely because a vendor protocol exposes the function. Verification status comes from the controlled BlueSky verification/configuration record.

## 10. Equipment terminology

The canonical term is **Equipment**.

The adapter contract shall not introduce `Payload` as a domain object, capability category or interface primitive.

If an external protocol uses the term `payload`, the adapter may retain that spelling only inside the external-protocol mapping and shall normalize it to the BlueSky `Equipment` model at the boundary.

## 11. Configuration and versioning

The adapter shall report the versions necessary to reproduce an integration decision:

```text
vehicle profile version
equipment profile version
adapter version
firmware version
configuration baseline version
protocol/schema version
```

A material compatibility change invalidates the affected verification state until revalidated.

## 12. Command boundary

```text
MISSION / FLIGHT LOGIC
        ↓
CANONICAL COMMAND
        ↓
SAFETY / AUTHORITY CHECK
        ↓
ADAPTER
        ↓
EXTERNAL PROTOCOL
```

The adapter is not an authorization layer and shall not bypass BlueSky safety or execution authority.

## 13. Telemetry boundary

```text
EXTERNAL TELEMETRY
        ↓
ADAPTER VALIDATION
        ↓
NORMALIZATION
        ↓
CANONICAL TELEMETRY
        ↓
STATE / SAFETY / HMI / LOGGING
```
