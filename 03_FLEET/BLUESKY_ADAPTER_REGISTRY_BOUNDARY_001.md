---
id: BLUESKY-ADAPTER-REGISTRY-BOUNDARY-001
type: adapter_registry_boundary
status: controlled_working_draft
system: BlueSky PRO
contract: BLUESKY-UNIVERSAL-ADAPTER-CONTRACT-001
schema: BLUESKY-CANONICAL-VEHICLE-EQUIPMENT-SCHEMA-001
---

# BlueSky PRO — Adapter Registry Boundary 001

## 1. Purpose

Define the smallest deterministic registry boundary required for BlueSky core services to resolve a supported Vehicle / Equipment adapter without depending on a concrete external protocol implementation.

This boundary is an integration contract, not a vendor selection and not evidence of concrete adapter conformance.

## 2. Boundary

```text
CORE SERVICE
    ↓
AdapterId / VehicleProfileId / EquipmentProfileId
    ↓
SERVICE-FACING ADAPTER RESOLVER
    ↓
ADAPTER REGISTRY
    ↓
Universal Adapter Contract
    ↓
CONCRETE ADAPTER
    ↓
EXTERNAL PROTOCOL
```

Core services shall resolve adapters through the service-facing resolver and registry boundary. They shall not instantiate or reference vendor-specific adapters directly.

## 3. Minimal registry record

Each registered adapter entry shall provide:

```text
adapter_id
adapter_version
vehicle_profiles[]
equipment_profiles[]
protocol
contract_id
schema_version
configuration_baseline_id
verification_state
lifecycle_state
```

## 4. Resolution result

The current technology-neutral implementation exposes the following deterministic result vocabulary:

```text
RESOLVED
NOT_FOUND
INCOMPATIBLE
CAPABILITY_UNSUPPORTED
AMBIGUOUS
```

`RESOLVED` means only that exactly one compatible registry entry was found. It does not mean the concrete adapter has passed operational verification.

`NOT_FOUND` means no registered adapter matches the requested target profile.

`INCOMPATIBLE` means a target adapter exists but the requested protocol, contract version or schema version is incompatible.

`CAPABILITY_UNSUPPORTED` means target/profile compatibility exists but the requested capability is not provided by the available candidate set.

`AMBIGUOUS` means more than one otherwise compatible adapter remains; the registry shall not select one arbitrarily.

## 5. Service-layer rule

The service-facing resolver is a thin technology-neutral façade. It delegates adapter resolution to the registry and introduces no vendor-specific selection or execution authority.

```text
Service
  ↓
AdapterServiceResolver.resolve()
  ↓
Registry.resolve()
  ↓
Adapter Contract
  ↓
Adapter operation
```

The registry owns lookup and compatibility resolution. It does not own safety authorization, mission release or flight execution authority.

## 6. Verification boundary

The registry shall preserve the distinction:

```text
REGISTERED
≠
CONTRACT-CONFORMING
≠
VERIFIED
≠
OPERATIONALLY APPROVED
```

Verification state is supplied by the controlled configuration/conformance records and must not be inferred from registration alone.

## 7. Initial controlled fixture

```text
adapter_id: TEST-ADAPTER-001
vehicle_profile: VEH-TEST-001
equipment_profile: EQ-TEST-001
protocol: MOCK-C2-EQUIPMENT-1
verification_state: NOT_VERIFIED
lifecycle_state: STUB
```

This fixture is synthetic and shall not be treated as real integration evidence.

## 8. Transition to concrete implementation

```text
REGISTRY BOUNDARY
→ concrete adapter registration
→ contract conformance execution
→ configuration verification
→ HIL / real hardware testing where applicable
→ operational verification
```

The registry boundary remains stable while concrete adapters are added.

## 9. Current status

```text
Registry boundary: DEFINED
Service-facing resolver: IMPLEMENTED
Service-facing resolver test: PASS in CI for the corresponding main commit
Concrete vendor adapter: NOT SELECTED
Synthetic fixture: DEFINED
Real hardware: NOT CONNECTED
Evidence: NOT AVAILABLE
```

**Status: CONTROLLED WORKING DRAFT — REGISTRY AND SERVICE-FACING RESOLUTION BOUNDARIES ALIGNED WITH CURRENT TECHNOLOGY-NEUTRAL IMPLEMENTATION.**
