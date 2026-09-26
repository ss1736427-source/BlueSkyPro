# BlueSky PRO — Controlled Capability Map

## Purpose

This document gives specialists a high-level capability map without exposing the implementation recipe.

| Capability | Review-level coverage | Current status |
|---|---|---|
| Product lifecycle | Mission through operational records and learning | Defined |
| Mission model | Common representation of mission context | Defined |
| Flight planning | Constraint-, environment- and energy-aware planning direction | Defined / development |
| Multi-UAV | Coordinated multi-aircraft planning workstream | Defined / development |
| Flight Chart / HMI | Map-first operational workspace | Development / refinement |
| C2 / telemetry | Communications and operational data boundaries | Defined / development |
| Vehicle / payload integration | Aircraft and payload abstraction boundaries | Defined |
| ATM / regulatory | Regulatory and authorization integration direction | Defined |
| Operational validation | Technical validation and readiness gates | Defined / development |
| AI / analytics | Prediction, operational learning and analysis | Defined / experimental |
| Assurance / traceability | Requirements, verification and evidence structure | Defined |
| Certification readiness | Engineering preparation for certification work | Working draft |
| Aircraft-level risk / insurance | Structured risk/evidence direction | Defined |
| Documentation / journal | Flight records and audit trail | Defined |
| CI / automated validation | Automated architecture/validator checks in repository | Implemented / experimental |

## Status note

This table is a review classification, not a product certification or acceptance statement.

The detailed repositories contain additional implementation and verification material that is intentionally not summarized here when doing so would materially improve reproducibility.

## Architecture at review level

```
Operational Context
       ↓
Mission
       ↓
Aircraft / Fleet
       ↓
Planning
       ↓
Verification
       ↓
Authorization / Operational Workflow
       ↓
Execution
       ↓
Records / Evidence
       ↓
Controlled Learning
```

The exact internal orchestration, object model, dependency graph and implementation sequence remain protected.

## Disclosure boundary

Do not expose in the review package:

- proprietary optimization formulas;
- numerical coefficients or calibration values;
- model weights or private datasets;
- internal executable validators;
- sensitive internal schemas when combined reproduction becomes possible;
- credentials, keys or production secrets;
- unreleased implementation decisions.
