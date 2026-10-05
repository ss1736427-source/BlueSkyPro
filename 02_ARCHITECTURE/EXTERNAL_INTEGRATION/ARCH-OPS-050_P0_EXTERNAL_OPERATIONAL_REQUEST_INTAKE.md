# ARCH-OPS-050 — External Operational Request Intake

**Status:** IMPLEMENTATION_BASELINE

## Objective

Create the single intake boundary through which an external operational request enters the existing Operational Orchestrator Pipeline.

## Flow

external operational request → request intake → Operational Orchestrator Pipeline → selected executable action → ARCH-OPS-049 execution router

## Rules

1. request_id is mandatory.
2. action_id is mandatory.
3. device_id is explicit and is never inferred from vehicle_id.
4. Required capabilities are explicit.
5. The request is evaluated through run_orchestrator_pipeline().
6. Runtime, safety, regulatory, energy, authority, and automation semantics remain owned by existing orchestrator components.
7. The requested action must be the action actually selected by the pipeline.
8. A human-required, runtime-blocked, or non-selected action is rejected at intake.
9. Intake performs no mission transfer and no command dispatch.
10. Execution remains downstream in ARCH-OPS-049.

## Boundary

The intake normalizes the external request envelope and submits it to the existing decision pipeline. It does not create a second authority gate, safety model, action selector, or execution path.

## Evidence

Target: SIL_EXTERNAL_OPERATIONAL_REQUEST_INTAKE

## Closure

Accepted intake results are bound to ARCH-OPS-049 by ARCH-OPS-051. The external request intake chain is complete.

Bind accepted intake results to the existing ARCH-OPS-049 execution router for a complete external-request-to-execution path.
