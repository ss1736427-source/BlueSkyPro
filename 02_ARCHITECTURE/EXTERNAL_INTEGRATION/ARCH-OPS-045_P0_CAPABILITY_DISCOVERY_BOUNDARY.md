# ARCH-OPS-045 — Capability Discovery Boundary

**Status:** IMPLEMENTATION_BASELINE

## Objective

Create the deterministic boundary between device identity/session admission and the normalized device capability runtime.

## Flow

`MAVLink/vendor discovery input → identity admission → capability snapshot → capability runtime`

The boundary accepts an already decoded discovery report. Transport and protocol decoders remain outside this component.

## Admission rules

1. The device identity must already exist.
2. A revoked identity cannot publish capabilities.
3. The discovery report device ID must match the admitted identity.
4. Empty capability identifiers are rejected.
5. Duplicate capabilities are normalized by the capability runtime.
6. A first accepted report registers the capability snapshot.
7. A later accepted report explicitly updates the existing snapshot.
8. The boundary does not authenticate credentials or interpret vendor-specific messages.

## Evidence target

`SIL_CAPABILITY_DISCOVERY_BOUNDARY`

## Closure

Capability compatibility is provided by ARCH-OPS-046 and consumed by ARCH-OPS-047. The historical next-gap statement is closed.

Use the admitted capability snapshot as a prerequisite for mission/command compatibility resolution.
