# PH5-AP-002 — PX4 Reference Adapter

**Priority:** P0  
**Status:** IMPLEMENTED — reference adapter boundary  
**Scope:** PX4 over MAVLink2

## Purpose

Provide the first executable PX4 reference adapter behind the existing Universal Autopilot Adapter contract.

## Delivered

- normalized PX4 identity and MAVLink2 protocol reporting;
- capability manifest inherited from the universal adapter boundary;
- deterministic compatibility resolution through the universal contract;
- contract test coverage registered in the Autopilot Adapter CMake target.

## Boundary

The reference adapter does not introduce MAVLink wire-message or transport dependencies into BlueSky core. Real transport, protocol services, SIL/HIL and real-UAV verification remain separate integration stages.

No new flight-control semantics, Energy Model parameters, C2 ownership or UI semantics are introduced.

## Verification state

`IMPLEMENTED` at contract/test level. SIL, HIL, real-UAV and product-readiness verification remain pending.
