# ARCH-OPS-019 — P0 Command Lifecycle Runtime

**Status:** IMPLEMENTATION BASELINE — SIL/CTest  
**Gap:** G0-03 Command protocol  
**Requirement:** SYS-AUT-006  
**Contract:** IF-COMMAND / BLUESKY-COMMAND-LIFECYCLE-CONTRACT-001

## Scope

This slice closes the smallest runtime gap between the canonical command lifecycle contract and a concrete deterministic implementation.

Implemented:
- canonical command identity validation;
- explicit lifecycle state transitions;
- deterministic dispatch attempt counting;
- bounded retry policy;
- retry restricted to explicitly idempotent commands;
- deterministic timeout evaluation;
- explicit terminal outcomes;
- explicit UNKNOWN for unresolved/link-loss outcomes;
- transition guards preventing invalid lifecycle jumps.

## Deliberate boundary

This is a protocol-independent runtime state machine. It does not implement:
- MAVLink/serial/UDP/TCP transport;
- vehicle ACK parsing;
- physical command dispatch;
- autopilot-specific semantics;
- authority/safety decisions;
- real-UAV/HIL evidence.

The authority/safety gate remains an upstream boundary. Transport and vehicle execution remain downstream integration work.

## State model

REQUESTED → VALIDATING → DISPATCHED → ACKNOWLEDGED → EXECUTING → COMPLETED

Failure branches remain explicit: REJECTED, FAILED, CANCELLED, TIMEOUT, UNKNOWN.

## Retry rule

Retries are bounded by max_retries and are allowed only when the command is explicitly marked idempotent. A non-idempotent command is never automatically retried by this runtime.

## Verification

command_lifecycle_runtime_test deterministically covers:
1. valid lifecycle progression;
2. attempt counting;
3. timeout boundary;
4. bounded idempotent retries;
5. completion after acknowledgement;
6. invalid command rejection;
7. non-idempotent retry rejection;
8. unresolved link-loss transition.

Evidence class remains software/SIL fixture evidence only.
