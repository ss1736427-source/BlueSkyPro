---
id: HAZARD-LOG-001
type: hazard_log
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Hazard Log

## 1. Назначение

Контролируемый реестр опасностей авиационной системы BlueSky.

Hazard Log связывает:

```text
Hazard
→ Cause / Condition
→ Effect
→ Safety Objective
→ Safety Requirement
→ Mitigation
→ Verification
→ Evidence
→ Residual Risk
```

Этот документ не устанавливает окончательные severity/likelihood/risk levels без утверждённой методики safety assessment.

## 2. Status values

```text
OPEN
UNDER ANALYSIS
MITIGATION DEFINED
VERIFICATION PENDING
VERIFIED
ACCEPTED
CLOSED
BLOCKED
```

## 3. Hazard record structure

Каждая запись должна содержать:

```text
Hazard ID
System / Function
Description
Cause / Condition
Effect
Operational Context
Severity
Likelihood / Probability
Risk Classification
Safety Objective
Safety Requirement IDs
Mitigation
Responsible Function
Verification Case IDs
Evidence IDs
Residual Risk
Acceptance Authority
Status
Change History
```

## 4. Preliminary hazard classes

```text
H-NAV   Navigation
H-RTE   Route Planning
H-WP    Waypoint
H-RET   Dynamic Return
H-COL   Collision Avoidance
H-C2    Communication / C2
H-MUL   Multi-UAV
H-MIS   Mission Execution
H-DATA  Data Quality
H-AUTH  Authorization
H-AI    AI-assisted functions
H-CFG   Configuration
H-SW    Software
H-HMI   Human Interface
```

## 5. Initial hazard register

| Hazard ID | Function | Preliminary hazard | Potential consequence | Related safety requirement | Status |
|---|---|---|---|---|---|
| H-NAV-001 | Navigation | Incorrect navigation state | Route deviation / unsafe flight | SAF-REQ-001, 007 | OPEN |
| H-NAV-002 | Navigation | Navigation degradation not detected | Incorrect safety decision | SAF-REQ-003, 007 | OPEN |
| H-RTE-001 | Route Planning | Unsafe route accepted | Unsafe trajectory | SAF-REQ-008 | OPEN |
| H-WP-001 | WP | Incorrect active WP | Wrong trajectory | TBD | OPEN |
| H-RET-001 | Dynamic Return | Return feasibility overestimated | Failure to return safely | SAF-REQ-009, 010 | OPEN |
| H-COL-001 | Collision Avoidance | Collision avoidance failure | Loss of separation / collision | SAF-REQ-011 | OPEN |
| H-C2-001 | C2 | C2 loss/degradation not handled | Loss of required response | SAF-REQ-013 | OPEN |
| H-MUL-001 | Multi-UAV | Incorrect individual state | Separation / coordination failure | SAF-REQ-012 | OPEN |
| H-DATA-001 | Data | Conflicting sources produce false state | Unsafe decision | SAF-REQ-014 | OPEN |
| H-AUTH-001 | Authorization | Unauthorized execution | Unsafe action | SAF-REQ-006 | OPEN |
| H-AI-001 | AI | AI proposal bypasses authority | Unsafe execution | SAF-REQ-018 | OPEN |
| H-CFG-001 | Configuration | Wrong configuration used | Invalid/unsafe behaviour or evidence | SAF-REQ-017 | OPEN |
| H-SW-001 | Software | Software failure affects safety function | Incorrect system behaviour | TBD | OPEN |
| H-HMI-001 | HMI | Operator receives/acts on incorrect safety information | Incorrect decision | TBD | OPEN |

## 6. Hazard analysis rule

Для каждой опасности необходимо отдельно установить:

```text
CAUSE
→ HAZARD
→ EFFECT
```

Не объединять причину, опасность и последствие в одно описание.

Пример:

```text
Cause:
stale navigation position

Hazard:
system uses an invalid representation of current position

Effect:
route deviation is not detected / unsafe navigation decision
```

## 7. Risk assessment

Окончательная оценка должна выполняться после определения применимой российской methodology/certification basis.

Пока не фиксируются:

- severity classes;
- probability values;
- exposure model;
- risk matrix;
- acceptance criteria.

До утверждения методики используются:

```text
UNKNOWN
TBD
```

А не выдуманные численные значения.

## 8. Safety objective linkage

Каждый hazard после анализа получает:

```text
Safety Objective
```

который затем декомпозируется в:

```text
Safety Requirement
→ Design Mitigation
→ Verification
```

## 9. Mitigation categories

Возможные mitigation layers:

```text
PREVENTION
DETECTION
MONITORING
VALIDATION
REJECTION / BLOCKING
DEGRADATION
RECOVERY
REDUNDANCY
PROCEDURAL CONTROL
OPERATOR ACTION
```

Mitigation не считается эффективной до verification.

## 10. Navigation hazards

Особое внимание:

```text
Incorrect Position
Stale Position
Invalid Position
Conflicting Position Sources
Incorrect Reference Frame
Course / Heading / Track confusion
Incorrect Wind
Incorrect WCA
Incorrect Track
Incorrect Groundspeed
Undetected Route Deviation
Incorrect WP transition
```

Каждая из этих ситуаций должна быть либо отдельной hazard, либо обоснованно объединена в hazard analysis.

## 11. Route / WP hazards

Проверяются:

```text
Wrong route version
Wrong active WP
WP transition failure
Invalid route geometry
Unsafe route modification
Route modification without revalidation
Map / WP / route inconsistency
```

## 12. Dynamic Return hazards

Проверяются:

```text
Return feasibility overestimated
Energy estimate incorrect
Wind change ignored
Groundspeed degradation ignored
Navigation degradation ignored
C2 degradation ignored
Return route invalid
Return decision based on stale state
```

## 13. Collision hazards

Проверяются:

```text
Detection failure
Tracking failure
Prediction failure
Data latency
Conflicting tracks
Incorrect separation assessment
Late avoidance
Avoidance command rejected
Avoidance conflict with mission/safety constraints
```

## 14. C2 hazards

Проверяются:

```text
Loss of communication
Degraded communication
Incorrect communication state
Stale C2 state
Unauthorized command
Command integrity failure
Failure to enter defined degraded response
```

## 15. Multi-UAV hazards

Проверяются:

```text
State mix-up
Wrong UAV identity
Incorrect relative state
Loss of one UAV state
Conflicting fleet information
Unsafe coordination
Collision caused by coordination failure
```

## 16. AI hazards

Проверяются:

```text
Incorrect recommendation
Hallucinated / unsupported information
Use of stale information
Incorrect confidence
Proposal bypasses validation
Proposal bypasses Safety Gate
Proposal becomes unauthorized execution
```

AI hazard analysis must distinguish:

```text
AI OUTPUT
≠
AUTHORITATIVE SYSTEM STATE
≠
EXECUTION AUTHORITY
```

## 17. Configuration hazards

Проверяются:

```text
Wrong software version
Wrong route/configuration
Unverified parameter set
Evidence generated against wrong configuration
Configuration drift
Uncontrolled change
```

## 18. Verification linkage

Hazard closure requires:

```text
Hazard
→ Safety Requirement
→ Verification Case
→ Result
→ Evidence
```

No hazard is considered closed merely because a mitigation exists in source code.

## 19. Residual risk

After verification:

```text
Initial Risk
→ Mitigation
→ Verified Effectiveness
→ Residual Risk
→ Acceptance
```

Residual risk requires explicit acceptance according to the approved safety methodology.

## 20. Change impact

Changes to any safety-significant function require hazard impact assessment:

```text
Navigation
Route Planning
Dynamic Return
Collision Avoidance
C2
Mission Execution
Multi-UAV
Safety Gate
Authorization
AI authority
Configuration
```

The analysis determines whether existing hazards, mitigations or verification cases are affected.

## 21. Evidence

Each accepted hazard shall reference controlled evidence:

```text
Safety Analysis
Test
Simulation
Inspection
Demonstration
Configuration Audit
Traceability Review
```

## 22. Current gaps

```text
HZ-GAP-001  Formal hazard analysis not completed
HZ-GAP-002  Risk methodology not approved
HZ-GAP-003  Severity classification not approved
HZ-GAP-004  Likelihood methodology not approved
HZ-GAP-005  Safety objectives not fully allocated
HZ-GAP-006  Safety requirements not fully traced
HZ-GAP-007  Verification cases not established
HZ-GAP-008  Evidence index not established
HZ-GAP-009  Residual-risk acceptance authority not defined
HZ-GAP-010  Certification-basis clause mapping pending
```

## 23. Required next step

После формирования Hazard Log необходимо:

```text
HAZARD LOG
→ SAFETY OBJECTIVES
→ SAFETY REQUIREMENTS
→ FORMAL RISK ASSESSMENT
→ SAFETY CASE
→ VERIFICATION
→ EVIDENCE
```

## 24. Status

**DRAFT_FOR_AGREEMENT**

Это первоначальный controlled hazard register. Окончательные risk levels, acceptance criteria и assurance levels устанавливаются после определения применимой certification basis и safety methodology.
