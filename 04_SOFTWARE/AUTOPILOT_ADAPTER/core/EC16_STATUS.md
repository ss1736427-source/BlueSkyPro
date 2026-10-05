# EC-16 — External Integration Adapter

Status: **PARTIAL — implementation baseline**

## Scope

Defines a universal adapter boundary for external-system exchange records. The baseline captures interface identity, message identity/type, direction, state, timestamp, payload reference/size, quality, validity and stale-data status.

## Safety boundary

This component records and validates exchange facts. It does **not** grant operational authorization, replace an external authority, or constitute physical certification evidence.

## Evidence

The implementation is compatible with the common verification/evidence architecture. Actual external-system records, controlled executions, raw source preservation and certification evidence remain required before EC-16 can be closed.

## Verification baseline

The unit test covers accepted valid input and rejection of invalid and stale messages.
