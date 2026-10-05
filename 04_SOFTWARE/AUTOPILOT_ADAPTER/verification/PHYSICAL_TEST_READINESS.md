# BlueSky PRO — Physical Test Readiness

**Status:** PROTOTYPE BASELINE

## Purpose

Define the preparation boundary for future physical verification without making physical-test results part of the prototype.

## Operational principle

The prototype collects and preserves facts through a stable source boundary. A real source can replace a demonstration source without changing the evidence package format or traceability identifiers.

## Data path

```text
Autopilot / C2 / Equipment / Instrument / GNSS / Weather
                         |
                         v
                 Source Adapter
                         |
                         v
                Common Evidence Event
                         |
                         v
                  Evidence Recorder
                         |
              +----------+----------+
              |                     |
              v                     v
          Raw events           Test Record
              |                     |
              +----------+----------+
                         v
                 Evidence Package
                         |
                         v
              Certification Export
```

## Prototype readiness criteria

Before physical tests, the prototype shall demonstrate:

1. A test run can be created with a unique `test_run_id`.
2. Each event contains source timestamp, parameter, value, unit and source identifier.
3. Source-specific data is normalized by a `Source Adapter` without changing the source fact.
4. Raw event data is retained separately from processed data.
5. A test can reference requirements, method, case, configuration and flight records.
6. The finalized package contains a record, raw events, tabular export, supporting evidence area, manifest and integrity anchor.
7. File integrity can be checked using SHA-256.
8. Demonstration/replay/development data is explicitly distinguished from physical verification data.
9. The resulting evidence package can be exported without manual re-entry of collected measurements.

## Physical-test transition

For a physical test, the test team supplies the approved test method and configuration. Real source adapters are then connected to the same common evidence interface.

The recorder does not determine regulatory acceptance. The approved test procedure and competent verification/certification process determine whether the collected evidence demonstrates the requirement.

## Evidence retention

Original source files and independently generated measurements are retained as supporting evidence. Processed tables and reports reference the same test identity and configuration so the result can be traced back to the underlying data.
