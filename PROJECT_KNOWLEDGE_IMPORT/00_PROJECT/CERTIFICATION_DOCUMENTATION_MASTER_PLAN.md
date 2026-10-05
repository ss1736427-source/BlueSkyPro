---
id: CERTIFICATION-DOCUMENTATION-MASTER-PLAN-001
type: certification_master_plan
status: draft_for_agreement
---

# Certification Documentation Master Plan

## 1. Objective

Подготовить проект так, чтобы документация и доказательства могли быть приведены к требованиям конкретного органа сертификации, а не собраны постфактум.

## 2. Target certification architecture

```text
REGULATORY FRAMEWORK
        ↓
CERTIFICATION PATH
        ↓
CERTIFICATION BASIS
        ↓
REQUIREMENTS BASELINE
        ↓
SYSTEM / SAFETY ARCHITECTURE
        ↓
SOFTWARE / HARDWARE / INTERFACE DESIGN
        ↓
IMPLEMENTATION
        ↓
VERIFICATION
        ↓
COMPLIANCE EVIDENCE
        ↓
CERTIFICATION PACKAGE
```

## 3. Documentation tree to establish

```text
00_PROJECT/
├── REGULATORY_KNOWLEDGE_BASE.md
├── CERTIFICATION_DOCUMENTATION_MASTER_PLAN.md
├── CERTIFICATION_STRATEGY.md
├── CERTIFICATION_BASIS.md
├── REGULATORY_SOURCE_REGISTER.md
├── COMPLIANCE_MATRIX.md
├── MEANS_OF_COMPLIANCE_MATRIX.md
├── CERTIFICATION_REQUIREMENTS_BASELINE.md
├── SAFETY_CASE.md
├── HAZARD_LOG.md
├── REQUIREMENTS_TRACEABILITY_MATRIX.md
├── VERIFICATION_EVIDENCE_INDEX.md
├── CONFIGURATION_BASELINE.md
├── CHANGE_AND_PROBLEM_PROCESS.md
└── CERTIFICATION_GAPS.md
```

Then controlled technical documentation:

```text
02_SYSTEM/
├── Requirements/
├── Architecture/
├── Safety/
├── Design/
├── Interfaces/
├── Software/
├── Verification/
└── Operations/
```

## 4. First certification deliverables

### A. Certification Strategy

Defines:

- authority;
- jurisdiction;
- certification/authorisation route;
- applicant;
- product/system scope;
- operational scope;
- target approval;
- certification milestones.

### B. Certification Basis

Defines the exact rules/standards/conditions against which compliance will be shown.

### C. Compliance Matrix

Minimum columns:

```text
Requirement ID
Source
Clause
Applicability
Project Requirement
Design Evidence
MoC
Verification Method
Evidence
Status
Authority Finding
```

### D. Means of Compliance Matrix

Methods:

```text
TEST
ANALYSIS
INSPECTION
SIMULATION
DEMONSTRATION
REVIEW OF DESIGN
COMBINATION
```

### E. Requirements Traceability Matrix

```text
Regulatory Requirement
→ System Requirement
→ Subsystem Requirement
→ Software Requirement
→ Design Element
→ Verification Case
→ Evidence
```

### F. Safety Case

```text
Hazard
→ Cause
→ Effect
→ Risk
→ Safety Objective
→ Mitigation
→ Requirement
→ Implementation
→ Verification
→ Residual Risk
```

## 5. BlueSky-specific certification questions

These must be answered before final classification:

1. Is BlueSky part of the aircraft/UAS or a ground control/mission system?
2. Does it generate or modify flight-critical navigation commands?
3. Does it directly control the aircraft?
4. Is the C++ Core airborne, ground-based, or both?
5. Which functions are safety-significant?
6. Which functions can affect separation/collision avoidance?
7. Which functions affect return-to-home / emergency behavior?
8. Which functions affect mission authorization?
9. Which functions are advisory only?
10. What happens after loss of communication?
11. What happens after loss/degradation of navigation?
12. How are software versions controlled?
13. How are mission data and user-modifiable data controlled?
14. How is configuration integrity demonstrated?
15. What evidence is retained for every safety-significant release?

## 6. Current certification gap assessment

### Already relatively strong

- architecture and responsibility boundaries;
- navigation knowledge chain;
- navigation state model;
- navigation rules;
- logical navigation algorithm;
- verification model;
- Safety Gate concept;
- AI authority boundary;
- route/WP concepts;
- Dynamic Return;
- UAV state/telemetry;
- Multi-UAV coordination.

### Needs certification conversion

- requirements;
- safety requirements;
- formal hazard analysis;
- regulatory traceability;
- compliance evidence;
- verification evidence;
- configuration management;
- change/problem reporting;
- software lifecycle plans;
- interface control;
- operational documentation;
- certification basis.

### Missing / priority

```text
Certification Strategy
Certification Basis
Regulatory Source Register
Compliance Matrix
Means of Compliance Matrix
Certification Requirements Baseline
Safety Case
Hazard Log
Requirements Traceability Matrix
Verification Evidence Index
Configuration Baseline
Certification Gap Register
```

## 7. Important rule

No project document should claim:

> "certification compliant"

until:

```text
Applicable Requirement
+
Accepted Certification Basis
+
Defined MoC
+
Completed Evidence
+
Authority Acceptance where required
```

are established.

## 8. Current status

**DRAFT_FOR_AGREEMENT**

The project is now being reorganised around certification readiness.

Next sequence:

```text
Regulatory Knowledge Base
→ Certification Strategy
→ Certification Basis
→ Compliance Matrix
→ Requirements Baseline
→ Safety Case
→ Traceability
→ Evidence
→ Certification Package
```



