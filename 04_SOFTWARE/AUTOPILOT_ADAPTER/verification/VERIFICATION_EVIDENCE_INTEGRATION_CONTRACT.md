# BlueSky PRO — Verification & Evidence Integration Contract

**Status:** IMPLEMENTATION BASELINE — P0
**Purpose:** define the integration boundary between operational BlueSky data, verification records and certification evidence so that prototype development can proceed before physical testing.

## 1. Engineering objective

The verification/evidence mechanism is an infrastructure service of the prototype. It is not a separate manual reporting process.

During normal operation or a controlled test execution, relevant system facts can be recorded once and later reused for engineering analysis, verification records and certification-document preparation.

The mechanism must not require redesign when the prototype source is replaced by a real autopilot, C2 channel, installed equipment or external measurement instrument.

## 2. Common evidence identity

Every verification execution has a `test_run_id`.

Where applicable, it is linked to:

- `requirement_ids` — requirements being verified;
- `test_method_id` and revision — approved verification method;
- `test_case_id` and revision — execution scenario;
- `configuration_id` — exact system configuration;
- `flight_record_id` — related operational/flight record;
- source identifiers — autopilot, C2, equipment and measurement sources.

These identifiers are references. The evidence recorder does not become the owner of the underlying operational systems.

## 3. Source-to-evidence interface

```text
Operational source
    │
    │ timestamp + parameter + value + unit + source
    ▼
Source Adapter
    │
    ▼
Common Evidence Event
    │
    ├── Raw event stream
    ├── Operational/Flight Record reference
    ├── Processed engineering data
    └── Evidence package
```

**Source Adapter (адаптер источника)** converts a source-specific record into the common evidence event format. It does not define acceptance criteria and does not alter the original source record.

## 4. Source categories

| Source | Purpose | Typical examples |
|---|---|---|
| `AUTOPILOT_TELEMETRY` | autopilot facts | position, mode, attitude, status |
| `C2_LINK` | command-and-control channel | RSSI, SNR, latency, packet loss, link state |
| `EQUIPMENT` | installed equipment | equipment status, measured values, health |
| `GNSS_RTK_NTRIP` | navigation/reference facts | position, quality, corrections |
| `WEATHER` | environmental conditions | wind, temperature, pressure |
| `MEASUREMENT_INSTRUMENT` | independent measurements | calibrated test equipment results |
| `BLUESKY_EVENT` | system facts | command accepted, mode transition, internal event |
| `OPERATOR_EVENT` | factual human observation | test start/stop, anomaly note |

## 5. What is recorded

For every collected parameter the minimum record is:

- timestamp;
- parameter identifier/name;
- value;
- unit;
- source;
- quality/status when available;
- context when required.

For long-running tests, records are appended during execution. The source timestamp is preserved; if BlueSky assigns a local receipt timestamp, it is additional metadata rather than a replacement.

## 6. What is not recorded as a fact

The recorder must not convert an assumption, expected value or acceptance limit into a measured fact.

The following remain separate:

- measured value;
- calculated value;
- acceptance limit;
- engineering judgement;
- verification result;
- certification conclusion.

This prevents prototype/demo data from being mistaken for physical test evidence.

## 7. Flight Record relationship

A `Flight Record (полная запись полёта)` remains the operational history of a flight.

A `Test Record (запись испытания)` describes a verification execution.

A test may reference one or more Flight Records, and a Flight Record may be used as evidence for a test, but neither record is overwritten by the other.

## 8. Evidence package

The portable package is:

```text
TEST_RUN_ID/
  record.json
  events.jsonl
  processed.csv
  evidence/
  manifest.json
  manifest.sha256
  report.md
```

`evidence/` stores original supporting files supplied by source systems or the test team. The recorder preserves their original file names and calculates integrity information during package finalization.

## 9. Certification-document connection

The future certification export shall use references rather than manual re-entry wherever possible:

```text
Requirement
   ↓
Test Method / Test Case
   ↓
Test Record
   ↓
Raw Data + Supporting Evidence
   ↓
Processed Result
   ↓
Evidence Package
   ↓
Certification Report/Table Reference
```

The export mechanism must retain the identifiers needed to reconstruct this chain.

## 10. Prototype mode

Before physical testing, the same interface may be exercised with:

- synthetic data;
- replayed logs;
- bench data;
- SIL/HIL data;
- manually entered factual events.

Such records shall be clearly marked as demonstration, development, replay or test data and shall never be represented as real physical verification results unless an approved procedure has actually been executed.

## 11. Physical-test readiness

The prototype is considered ready for physical-test integration when a real source can be connected through a Source Adapter and provide data to the recorder without changing:

- the evidence directory structure;
- the common event format;
- requirement/test identifiers;
- traceability links;
- export structure;
- integrity mechanism.

## 12. Responsibility boundary

The software collects and preserves evidence.

The test procedure defines how the test is conducted and what constitutes acceptance.

The competent verification/certification process determines whether the evidence is sufficient and whether the requirement is demonstrated.

The prototype itself does not declare regulatory certification.
