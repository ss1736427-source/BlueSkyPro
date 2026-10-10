---
id: SAFETY-REQUIREMENTS-BASELINE-001
type: safety_requirements_baseline
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Safety Requirements Baseline

## 1. Назначение

Определить системный safety layer авиационной системы BlueSky как основу для последующего Hazard Log, Safety Case, allocation требований и verification.

Документ не назначает окончательный уровень критичности или risk class без завершения formal safety assessment и определения certification basis.

## 2. Safety principle

```text
HAZARD
 ↓
CAUSE / CONDITION
 ↓
EFFECT
 ↓
RISK
 ↓
SAFETY OBJECTIVE
 ↓
SAFETY REQUIREMENT
 ↓
MITIGATION
 ↓
DESIGN
 ↓
VERIFICATION
 ↓
EVIDENCE
```

Safety requirements имеют приоритет над mission optimisation.

## 3. System safety boundary

Безопасность рассматривается на уровне всей авиационной системы:

```text
Navigation
Mission
Route Planning
WP
Dynamic Return
Collision Avoidance
C2
Multi-UAV
Readiness
Safety Gate
Authorization
HMI
AI-assisted functions
Configuration
```

Safety significance каждой функции должна быть классифицирована отдельно.

## 4. Safety authority

Установленная архитектурная цепочка:

```text
DATA
 ↓
STATE
 ↓
VALIDATION
 ↓
READINESS
 ↓
SAFETY GATE
 ↓
AUTHORIZATION
 ↓
C++ CORE
 ↓
EXECUTION
```

Никакой optimisation, UI или AI proposal не должен обходить safety authority.

## 5. Safety requirement record

Каждая safety requirement должна иметь:

```text
Safety Requirement ID
Hazard ID
Safety Objective ID
Source
Rationale
System Allocation
Safety Classification
Verification Method
Evidence
Configuration
Status
```

## 6. Initial safety requirements

### SAF-REQ-001 — Validated state

Safety-significant decisions shall use a validated system state with explicit quality/provenance information.

**Status:** OPEN

**Verification:** Analysis / Test

---

### SAF-REQ-002 — Invalid data handling

Invalid safety-significant input shall not silently be treated as valid input.

**Status:** OPEN

**Verification:** Negative Test

---

### SAF-REQ-003 — Stale data handling

Stale safety-significant data shall be detected and its operational impact assessed.

Exact stale thresholds remain TBD.

**Status:** OPEN

**Verification:** Test / Analysis

---

### SAF-REQ-004 — Loss of critical data

Loss of required safety-significant input shall produce a defined degraded/failure response.

**Status:** OPEN

**Verification:** Failure Scenario Test

---

### SAF-REQ-005 — Safety Gate integrity

Safety Gate shall not be bypassed by mission optimisation, UI actions or AI-generated proposals.

**Status:** OPEN

**Verification:** Architecture Review / Negative Test

---

### SAF-REQ-006 — Authorization boundary

Safety-significant execution shall occur only through the defined authorization path.

**Status:** OPEN

**Verification:** Architecture Review / Integration Test

---

### SAF-REQ-007 — Navigation safety

Navigation degradation affecting safety shall be detected and propagated to the appropriate safety decision layer.

**Status:** OPEN

**Verification:** Test / Simulation

---

### SAF-REQ-008 — Route safety

A route or route modification that fails applicable safety validation shall not be authorized for execution.

**Status:** OPEN

**Verification:** Negative Test

---

### SAF-REQ-009 — Dynamic Return

Return feasibility shall be evaluated from the current state of the affected UAV and applicable constraints.

**Status:** OPEN

**Verification:** Analysis / Simulation / Test

---

### SAF-REQ-010 — Dynamic Return degradation

Material degradation of energy, wind, groundspeed, communication, navigation or other relevant conditions shall trigger reassessment of return feasibility.

**Status:** OPEN

**Verification:** Scenario Test

---

### SAF-REQ-011 — Collision avoidance

Safety-significant separation/collision avoidance functions shall have defined requirements, authority boundaries and verification evidence.

**Status:** OPEN

**Verification:** Simulation / Integration Test

---

### SAF-REQ-012 — Multi-UAV state isolation

Safety-relevant state of each UAV shall remain individually identifiable and shall not be corrupted by another UAV's state.

**Status:** OPEN

**Verification:** Integration Test

---

### SAF-REQ-013 — C2 degradation

Loss or degradation of C2 shall be detected and mapped to a defined safety response.

**Status:** OPEN

**Verification:** Failure Scenario Test

---

### SAF-REQ-014 — Conflicting sources

Conflicting safety-significant sources shall be detected and shall not silently produce an unsafe authoritative state.

**Status:** OPEN

**Verification:** Test / Analysis

---

### SAF-REQ-015 — Revalidation

A material safety-significant state change shall trigger the applicable revalidation process.

**Status:** OPEN

**Verification:** Scenario Test / Integration Test

---

### SAF-REQ-016 — Traceability

Every safety-significant requirement shall be traceable to its hazard/objective, implementation and verification evidence.

**Status:** OPEN

**Verification:** Traceability Review

---

### SAF-REQ-017 — Configuration control

Safety evidence shall be linked to the configuration under which it was produced.

**Status:** OPEN

**Verification:** Configuration Audit

---

### SAF-REQ-018 — AI boundary

AI-generated analysis or recommendation shall not independently authorize safety-significant execution where authorization is required.

**Status:** OPEN

**Verification:** Negative Test / Architecture Review

---

## 7. Preliminary hazard classes

Для первичного анализа используются:

```text
H-NAV   Navigation failure/degradation
H-RTE   Unsafe route
H-WP    WP / route transition error
H-RET   Unsafe return
H-COL   Collision / loss of separation
H-C2    C2 loss/degradation
H-MUL   Multi-UAV coordination failure
H-MIS   Mission execution failure
H-DATA  Invalid/stale/conflicting data
H-AUTH  Unauthorized execution
H-AI    Unsafe AI influence
H-CFG   Configuration error
H-SW    Software failure
H-HMI   Human interaction error
```

Это классы для анализа, а не окончательный hazard classification.

## 8. Initial hazard records

| Hazard ID | Preliminary hazard | Potential consequence | Status |
|---|---|---|---|
| H-NAV-001 | Incorrect navigation state | Route deviation / unsafe flight | OPEN |
| H-NAV-002 | Navigation data degradation not detected | Incorrect decision | OPEN |
| H-RTE-001 | Unsafe route accepted | Unsafe trajectory | OPEN |
| H-WP-001 | Incorrect active WP | Wrong trajectory | OPEN |
| H-RET-001 | Return feasibility overestimated | Failure to return safely | OPEN |
| H-COL-001 | Collision avoidance failure | Loss of separation / collision | OPEN |
| H-C2-001 | C2 loss not handled | Loss of control/mission response | OPEN |
| H-MUL-001 | Incorrect multi-UAV state | Separation failure | OPEN |
| H-DATA-001 | Conflicting sources produce false state | Unsafe decision | OPEN |
| H-AUTH-001 | Unauthorized execution | Unsafe action | OPEN |
| H-AI-001 | AI proposal bypasses authority | Unsafe execution | OPEN |
| H-CFG-001 | Wrong configuration used | Invalid safety evidence / behaviour | OPEN |

## 9. Risk assessment

Risk assessment shall not be invented in this baseline.

For each hazard, later determine:

```text
Severity
Likelihood / Probability
Exposure where applicable
Risk classification
Safety objective
Required assurance
Mitigations
Residual risk
Acceptance criteria
```

The applicable Russian risk methodology depends on the final operational/certification category.

## 10. Safety verification classes

```text
SV-1  Requirements Review
SV-2  Architecture Review
SV-3  Analysis
SV-4  Simulation
SV-5  Unit / Component Test
SV-6  Integration Test
SV-7  System Test
SV-8  Failure / Negative Test
SV-9  Scenario / Operational Demonstration
SV-10 Traceability Audit
SV-11 Configuration Audit
```

## 11. Safety evidence

Evidence must be controlled:

```text
Safety Requirement
→ Verification Case
→ Test / Analysis
→ Result
→ Configuration
→ Review
→ Evidence ID
```

## 12. Safety change impact

Any change to:

- Navigation;
- Route Planning;
- Dynamic Return;
- Collision Avoidance;
- C2;
- Mission Execution;
- Multi-UAV;
- Safety Gate;
- Authorization;
- AI authority;
- configuration;

requires safety impact assessment.

## 13. Certification linkage

Safety requirements shall eventually link to:

```text
Applicable Regulation / Standard
→ Certification Requirement
→ Safety Requirement
→ Hazard
→ Mitigation
→ Design
→ Verification
→ Evidence
```

## 14. Current gaps

```text
SAF-GAP-001  Formal FHA / hazard analysis not completed
SAF-GAP-002  Severity classifications not approved
SAF-GAP-003  Likelihood methodology not approved
SAF-GAP-004  Safety objectives not fully allocated
SAF-GAP-005  Required assurance levels not established
SAF-GAP-006  Safety evidence criteria not finalized
SAF-GAP-007  Certification basis clause mapping pending
SAF-GAP-008  Operational risk category pending
```

## 15. Next documents

```text
HAZARD_LOG.md
SAFETY_CASE.md
SAFETY_REQUIREMENTS_TRACEABILITY.md
```

## 16. Status

**DRAFT_FOR_AGREEMENT**

This baseline establishes the safety structure and preliminary requirements. It does not replace formal safety assessment or authority-approved certification requirements.



