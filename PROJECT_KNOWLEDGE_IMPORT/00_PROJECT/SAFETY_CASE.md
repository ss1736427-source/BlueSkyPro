---
id: SAFETY-CASE-001
type: safety_case
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Safety Case

## 1. Назначение

Safety Case является структурированной аргументацией того, что safety-significant функции авиационной системы имеют определённые опасности, требования, mitigations, verification и evidence.

Это рабочая основа. Она не является заявлением о сертификационном соответствии и не заменяет formal safety assessment или требования уполномоченного органа.

## 2. Safety argument

Основная аргументация:

```text
CLAIM
Авиационная система безопасно выполняет заявленные safety-significant функции
        ↓
SUB-CLAIM
Опасности системно выявлены
        ↓
SUB-CLAIM
Для опасностей определены safety objectives
        ↓
SUB-CLAIM
Safety requirements реализованы
        ↓
SUB-CLAIM
Mitigations проверены
        ↓
SUB-CLAIM
Verification evidence относится к контролируемой конфигурации
        ↓
SUB-CLAIM
Остаточный риск оценён и принят по утверждённой методике
```

Последний пункт остаётся открытым до утверждения risk methodology и acceptance authority.

## 3. Scope

Safety Case охватывает авиационную систему и следующие предварительно safety-significant области:

- Navigation;
- Route Planning;
- WP Management;
- Dynamic Return;
- Collision Avoidance;
- C2;
- Mission Execution;
- Multi-UAV;
- Readiness;
- Safety Gate;
- Authorization;
- HMI;
- AI-assisted functions;
- Configuration.

## 4. Safety authority model

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

Safety authority не должна зависеть от mission optimisation.

AI proposal и UI interaction не должны самостоятельно обходить safety gate и authorization boundary.

## 5. Top-level claims

### C-001 — System hazards are identified

**Argument:** Hazard Log содержит контролируемый перечень опасностей и правила их анализа.

**Evidence:**

`HAZARD_LOG.md`

**Status:** OPEN

---

### C-002 — Safety requirements are defined

**Argument:** Safety Requirements Baseline определяет первоначальные safety requirements и связывает их с safety functions.

**Evidence:**

`SAFETY_REQUIREMENTS_BASELINE.md`

**Status:** OPEN

---

### C-003 — Safety-significant functions are controlled

**Argument:** Safety-significant functions имеют определённые authority boundaries и должны проходить validation/revalidation.

**Evidence candidates:**

- `AVIATION_SYSTEM_DEFINITION.md`
- `SAFETY_REQUIREMENTS_BASELINE.md`
- system architecture
- verification evidence

**Status:** OPEN

---

### C-004 — Navigation safety is controlled

**Argument:** Navigation state, quality, reference frames, deviation and degradation are explicitly modeled.

**Evidence candidates:**

- `NAVIGATION_KNOWLEDGE_MAP.md`
- `NAVIGATION_STATE_MODEL.md`
- `NAVIGATION_RULES.md`
- `NAVIGATION_ALGORITHM.md`
- `NAVIGATION_VERIFICATION_MODEL.md`

**Status:** OPEN

---

### C-005 — Unsafe route execution is prevented

**Argument:** Route and route modifications require validation before safety-significant execution.

**Evidence candidates:**

- Route requirements
- Safety Gate design
- Negative tests
- Integration tests

**Status:** OPEN

---

### C-006 — Dynamic Return is safety-controlled

**Argument:** Return feasibility is based on current UAV state and applicable constraints, with reassessment after material changes.

**Evidence candidates:**

- Dynamic Return requirements
- simulation
- scenario tests
- system test evidence

**Status:** OPEN

---

### C-007 — Collision avoidance is safety-controlled

**Argument:** Collision avoidance has explicit requirements, authority boundaries and system-level verification.

**Evidence candidates:**

- Collision Avoidance requirements
- scenario simulations
- integration tests
- system demonstrations

**Status:** OPEN

---

### C-008 — C2 degradation is handled

**Argument:** C2 state and degradation have defined detection and response behaviour.

**Evidence candidates:**

- C2 requirements
- failure scenario tests
- system integration tests

**Status:** OPEN

---

### C-009 — Multi-UAV safety is controlled

**Argument:** Each UAV retains individually identifiable state and safety-significant coordination is verified at system level.

**Evidence candidates:**

- Multi-UAV requirements
- integration tests
- multi-UAV simulations

**Status:** OPEN

---

### C-010 — AI cannot silently become execution authority

**Argument:** AI output is separated from authoritative state and execution authority.

**Evidence candidates:**

- architecture
- AI boundary requirements
- negative tests
- authorization tests

**Status:** OPEN

---

### C-011 — Configuration is controlled

**Argument:** Safety evidence is associated with an identifiable system/software/configuration baseline.

**Evidence candidates:**

- Configuration Baseline
- Change Records
- Evidence Index
- Configuration Audit

**Status:** OPEN

## 6. Claim → evidence structure

For every claim:

```text
CLAIM
→ ARGUMENT
→ REQUIREMENTS
→ DESIGN
→ VERIFICATION
→ EVIDENCE
→ CONFIGURATION
→ STATUS
```

Evidence must be independently identifiable.

## 7. Safety requirements linkage

```text
SAF-REQ-001
→ Validated state
→ State validation implementation
→ Failure / negative tests
→ Evidence

SAF-REQ-005
→ Safety Gate integrity
→ Architecture / implementation
→ Bypass tests
→ Evidence

SAF-REQ-009
→ Dynamic Return feasibility
→ Return calculation
→ Scenario verification
→ Evidence

SAF-REQ-013
→ C2 degradation
→ C2 state handling
→ Failure scenario
→ Evidence

SAF-REQ-018
→ AI authority boundary
→ Authorization architecture
→ Negative test
→ Evidence
```

## 8. Hazard linkage

Example:

```text
H-NAV-001
Incorrect navigation state
        ↓
Safety Objective
        ↓
SAF-REQ-001 / SAF-REQ-007
        ↓
Navigation validation
        ↓
Verification
        ↓
Evidence
```

Another example:

```text
H-RET-001
Return feasibility overestimated
        ↓
Safety Objective
        ↓
SAF-REQ-009 / SAF-REQ-010
        ↓
Dynamic Return
        ↓
Scenario Verification
        ↓
Evidence
```

## 9. Evidence categories

```text
REQUIREMENT
ARCHITECTURE
DESIGN
ANALYSIS
SIMULATION
TEST
INSPECTION
DEMONSTRATION
CONFIGURATION
TRACEABILITY
OPERATIONAL
```

## 10. Evidence acceptance

Evidence must identify:

```text
Evidence ID
Source requirement
Verification method
System version
Software version
Configuration
Test environment
Scenario / dataset
Result
Reviewer
Date
Status
```

## 11. Negative assurance cases

Safety Case must explicitly demonstrate that unsafe paths are blocked.

Examples:

```text
Invalid navigation data
→ execution blocked / degraded response

Stale critical data
→ stale condition detected

Unsafe route
→ Safety Gate blocks authorization

AI unsafe proposal
→ proposal cannot bypass authority

Unauthorized execution request
→ authorization path rejects it

C2 loss
→ defined degraded response

Wrong configuration
→ configuration verification detects mismatch
```

## 12. Independence of evidence

Where required by the applicable certification basis, verification and review activities must have appropriate independence from the implementation activity.

Exact independence level remains TBD until certification basis and assurance classification are established.

## 13. Residual risk

The Safety Case cannot close until:

```text
Hazard
→ Risk Assessment
→ Mitigation
→ Verification
→ Residual Risk
→ Acceptance
```

Risk values and acceptance criteria are intentionally not invented here.

## 14. Certification linkage

Safety Case links to:

```text
REGULATORY_SOURCE_REGISTER
        ↓
CERTIFICATION_BASIS
        ↓
COMPLIANCE_MATRIX
        ↓
CERTIFICATION_REQUIREMENTS_BASELINE
        ↓
SAFETY_REQUIREMENTS_BASELINE
        ↓
HAZARD_LOG
        ↓
SAFETY_CASE
        ↓
VERIFICATION
        ↓
EVIDENCE
```

## 15. Current open issues

```text
SC-GAP-001  Formal safety assessment incomplete
SC-GAP-002  Risk methodology not approved
SC-GAP-003  Severity/likelihood classification pending
SC-GAP-004  Safety objectives not fully approved
SC-GAP-005  Safety assurance level pending
SC-GAP-006  Certification basis clause mapping pending
SC-GAP-007  Evidence index not established
SC-GAP-008  Configuration baseline not established
SC-GAP-009  Residual-risk acceptance authority not defined
SC-GAP-010  Independent verification requirements pending
```

## 16. Closure criteria

A top-level safety claim may move toward closure only when:

```text
Applicable requirements identified
+
Hazards analysed
+
Safety requirements approved
+
Mitigations implemented
+
Verification completed
+
Evidence controlled
+
Configuration identified
+
Residual risk assessed
+
Required acceptance obtained
```

## 17. Status

**DRAFT_FOR_AGREEMENT**

The Safety Case is the argument structure. It becomes certification evidence only after the underlying requirements, analyses, verification results and acceptance records are completed and controlled.



