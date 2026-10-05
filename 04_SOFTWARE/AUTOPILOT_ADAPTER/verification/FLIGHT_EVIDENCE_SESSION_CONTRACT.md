# Flight Evidence Session Contract

## Purpose

Define the common session that joins operational flight data with verification evidence without changing the raw source data.

## Session identity

Each session shall have:

- `session_id` — unique session identifier.
- `test_run_id` — verification run identifier when the session belongs to a test.
- `flight_record_id` — flight record identifier when a flight is involved.
- `mission_id` — mission identifier when a mission is involved.
- `configuration_id` — exact system configuration identifier.
- `started_at_utc` and `ended_at_utc` — session boundaries.

## Lifecycle

`INIT → COLLECT → FINALIZE → VERIFY → EXPORT → ARCHIVE`

## Source participation

A session may receive data from any registered evidence source. The source must be identified by `source_id` and mapped to an `evidence_domain_id`.

Minimum traceability for each evidence event:

`session_id → source_id → evidence_domain_id → timestamp_utc → parameter/value → raw record reference`

## Integrity

Raw records are retained unchanged. Derived values must identify their source record and processing step. Exported evidence shall include a manifest with file hashes.

## Prototype rule

Demonstration, development, replay and bench data may exercise the same pipeline but shall retain their declared evidence class and shall not be represented as physical test evidence automatically.

## Acceptance criterion

The session mechanism is considered operational when one session can correlate data from multiple registered domains and export a traceable evidence package without manual copying of individual measurements.
