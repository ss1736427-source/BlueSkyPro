# ARCH-OPS-043 — Device identity

## Requirement

G2-02 requires UAV/autopilot/component identity plus certificates/keys and lifecycle.

## Implemented slice

DeviceIdentityRuntime establishes a stable device identity boundary and credential fingerprint association. It provides deterministic registration, collision detection, resolution and lifecycle state.

## Authority boundaries

- Device identity is distinct from MAVLink session identity.
- Device identity is distinct from MAVLink system/component addressing.
- Credential fingerprints are references, not private key material.
- No live authentication or certificate validation is claimed by this slice.

## Evidence

SIL_DEVICE_IDENTITY_RUNTIME

The evidence demonstrates deterministic identity lifecycle behavior in-process. It does not demonstrate cryptographic validation, key provisioning, certificate-chain trust or field security.
