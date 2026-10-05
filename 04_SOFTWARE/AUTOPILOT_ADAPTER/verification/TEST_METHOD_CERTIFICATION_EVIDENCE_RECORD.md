# BlueSky PRO — Test Method & Certification Evidence Record

**Document class:** controlled template
**Status:** TEMPLATE — NO TEST RESULTS
**Purpose:** common record structure for future physical and simulated verification.

## A. Identification

- Test Run ID:
- Requirement ID(s):
- Test Method ID / Revision:
- Test Case ID / Revision:
- Date/time:
- Site:
- Responsible personnel:

## B. System configuration

- BlueSky version / commit:
- Vehicle ID / type:
- Autopilot / firmware:
- Adapter / protocol version:
- Equipment configuration:
- C2 configuration:
- Navigation configuration:
- Weather/environmental configuration:
- Measurement equipment and calibration references, if applicable:

## C. Objective

State the exact requirement or engineering question being verified.

## D. Preconditions

List all conditions that must be satisfied before execution. Record the actual condition values; do not replace them with assumptions.

## E. Procedure

Numbered actions with observable expected results. Any deviation is recorded as an event and linked to the affected step.

## F. Parameters to collect

For each parameter define before testing:

- parameter ID/name;
- unit;
- source;
- sampling/event rule;
- required quality flag;
- acceptance criterion reference;
- whether the value is raw or derived.

For a C2/link test, the parameter list may include latency, packet loss, RSSI, SNR, link state, reconnect time, channel/path identity, command acknowledgement and relevant timing/position information. The exact list is controlled by the approved test method and actual equipment capability.

## G. Evidence

Record the relative paths/identifiers of:

- raw telemetry/logs;
- C2/link logs;
- equipment logs;
- configuration snapshots;
- measurement files;
- screen/video records;
- generated analysis;
- generated report;
- integrity manifest.

## H. Result

Allowed states:

- `PENDING` — planned, not executed;
- `NOT_PERFORMED` — formally not performed;
- `IN_PROGRESS` — execution active;
- `ANALYZED` — data collected and under analysis;
- `PASS` — acceptance criteria demonstrated;
- `FAIL` — acceptance criteria not demonstrated;
- `INVALID` — evidence cannot support a valid conclusion.

A `PASS` or `FAIL` conclusion shall reference the actual data and applicable acceptance criteria.

## I. Deviation / anomaly

Every deviation, interruption, equipment fault or data-quality problem receives an identifier and disposition. The original evidence remains preserved.

## J. Certification linkage

- Requirement:
- Verification record:
- Evidence package:
- Analysis/report:
- Certification document/reference:
- Reviewer/approval:

## K. Evidence integrity

The generated `manifest.json` contains SHA-256 hashes for controlled files. The evidence package is considered incomplete if the manifest or required source data is missing.

## L. Important limitation

This template prepares the evidence structure. It does not itself establish regulatory compliance. Applicable requirements, approved procedures, acceptance limits and competent authority/certification decisions remain controlling.
