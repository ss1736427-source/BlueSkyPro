# ARCH-OPS-048 — Capability Execution Integration

Status: IMPLEMENTATION_BASELINE

## Objective

Connect the capability admission boundary to the existing mission transfer and command dispatch entry points so an incompatible action cannot enter execution.

## Runtime flow

Device Identity
→ Capability Discovery
→ Capability Runtime
→ Compatibility Gate
→ Admission Boundary
→ Mission Transfer / Command Dispatch

## Mission transfer

CapabilityExecutionIntegration::executeMission():

1. builds an admission request using the explicit device_id;
2. requires the declared mission capabilities;
3. rejects before MissionTransferRuntime::execute() when admission is not Admitted;
4. otherwise delegates to the existing mission transfer runtime unchanged.

The device_id is kept separate from vehicle_id.

## Command dispatch

CapabilityExecutionIntegration::dispatchCommand():

1. builds an admission request using the explicit device_id;
2. requires the command capability set;
3. rejects before CommandLifecycleRuntime::validate() / dispatch() when admission is not Admitted;
4. otherwise enters the existing command lifecycle at validation and dispatch.

## Boundary rules

- capability admission is mandatory before mission transfer or command dispatch;
- incompatible actions do not reach the execution entry point;
- existing mission transfer and command lifecycle semantics remain authoritative after admission;
- capability state is read-only at this boundary;
- no authentication, transport, MAVLink parsing, persistence, policy authorization, or command execution is implemented here;
- device_id is never inferred from vehicle_id.

## Evidence

Target: SIL_CAPABILITY_EXECUTION_INTEGRATION

Coverage includes:
- admitted command reaches dispatch;
- rejected command does not increment dispatch attempts;
- admitted mission passes the capability gate and reaches mission transfer;
- rejected mission is stopped before mission transfer.

## Closure

The guarded execution entry points are routed through ARCH-OPS-049, with external intake completed by ARCH-OPS-050/051.

Connect the admission-integrated entry points to the higher-level operational orchestrator / action routing path so all external mission and command requests use the same guarded entry points.
