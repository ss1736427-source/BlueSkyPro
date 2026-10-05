# Authorization-Qualified Constrained Environment

**ID:** PLAN-AUTH-ENV-001
**Status:** BASELINED

## Purpose

Provide one deterministic qualification result for a current constrained environment when a regulatory restriction may be operated under a current explicit authorization.

## Contract

The builder preserves the source environment and evaluates authorization coverage per restriction. A restriction is **Qualified** only when the authorization is present, valid, and its geographic/scope, time, altitude and operation conditions all match.

Qualified regulatory restrictions are marked non-blocking in the derived planning environment. The original restriction remains available through the qualified record for Flight Chart inspection and traceability.

Missing or non-matching authorization never removes the restriction. Physical constraints are not authorization-qualifiable by this contract.

## Consumers

The same qualified result is intended to feed:
- constrained spatial search;
- Flight Chart visualization state;
- authorization readiness.

No consumer shall independently reinterpret authorization against the same restriction.

## Dependency and recalculation

The result is versioned by source environment, calculation input and qualification contract version. A change in authorization or source environment invalidates the derived qualified environment and affected downstream calculations.

## Boundary

This contract does not grant authorization, submit a flight plan, alter authoritative airspace data, remove terrain/obstacle constraints, execute a flight, or establish overall mission READY status.
