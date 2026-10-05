# ARCH-OPS-049 — Operational Action Routing

Status: IMPLEMENTATION_BASELINE

## Objective

Make the Operational Orchestrator the single routing point from a selected executable action to the capability-guarded mission or command entry point.

## Flow

Runtime Context
→ Orchestrator Pipeline
→ Selected Executable Action
→ Action Execution Binding
→ Capability Admission
→ Mission Transfer / Command Dispatch

## Rules

- an action must be selected by the existing orchestrator pipeline;
- the selected action id must exactly match the execution binding;
- only an Executable selection is routable;
- mission and command routing are explicit kinds;
- device_id is explicit and is never inferred from vehicle_id;
- required capabilities are explicit in the binding;
- all routing enters ARCH-OPS-048 guarded entry points;
- incompatible actions are rejected before dispatch or mission transfer;
- the router does not redefine authority, safety, regulatory, energy, or runtime gates.

## Evidence

Target: SIL_OPERATIONAL_ACTION_EXECUTION_ROUTER

Coverage includes selected command routing, action mismatch rejection, and capability rejection before command dispatch.

## Closure

External operational request intake is implemented by ARCH-OPS-050 and bound to this router by ARCH-OPS-051. The routing chain is complete.

Bind external operational request intake to the orchestrator pipeline so action selection cannot be bypassed by direct callers.
