---
id: CHANGE-AND-PROBLEM-PROCESS-001
type: change_and_problem_process
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Change and Problem Process

## 1. Назначение

Установить единый контролируемый процесс управления изменениями, дефектами, несоответствиями и их влиянием на безопасность и сертификационные доказательства авиационной системы BlueSky.

## 2. Основной принцип

Ни одно существенное изменение не должно проходить напрямую из идеи в production/certification baseline.

```text
CHANGE / PROBLEM
→ IDENTIFICATION
→ CLASSIFICATION
→ IMPACT ASSESSMENT
→ DECISION
→ IMPLEMENTATION
→ VERIFICATION
→ EVIDENCE UPDATE
→ BASELINE UPDATE
```

## 3. Change categories

```text
CHG-REQ   Requirements
CHG-SYS   System architecture
CHG-SAF   Safety
CHG-NAV   Navigation
CHG-RTE   Route Planning
CHG-RET   Dynamic Return
CHG-COL   Collision Avoidance
CHG-C2    C2
CHG-MUL   Multi-UAV
CHG-HMI   HMI
CHG-AI    AI authority / behaviour
CHG-SW    Software
CHG-HW    Hardware
CHG-DATA  Data
CHG-CFG   Configuration
CHG-VER   Verification
CHG-CERT  Certification documentation
```

## 4. Problem categories

```text
PRB-DEFECT
PRB-FAILURE
PRB-NONCONFORMANCE
PRB-TEST-FAIL
PRB-REQUIREMENT
PRB-DOCUMENTATION
PRB-CONFIGURATION
PRB-SAFETY
PRB-CERTIFICATION
```

## 5. Change record

Каждое изменение должно иметь:

```text
Change ID
Title
Requester
Date
Affected CI
Reason
Description
Priority
Safety Impact
Certification Impact
Requirements Impact
Design Impact
Verification Impact
Evidence Impact
Configuration Impact
Decision
Approver
Implementation
Verification
Result
Baseline
Status
```

## 6. Problem record

Каждая проблема должна иметь:

```text
Problem ID
Description
Detection Source
Affected Configuration
Severity
Safety Impact
Certification Impact
Containment
Root Cause
Corrective Action
Verification
Evidence
Disposition
Status
```

## 7. Impact assessment

До approval оцениваются:

### Safety

```text
New hazard?
Existing hazard affected?
Safety requirement affected?
Mitigation affected?
Risk affected?
```

### Certification

```text
Certification requirement affected?
Certification basis affected?
Compliance matrix affected?
MoC affected?
Evidence invalidated?
Authority notification/approval potentially required?
```

### Requirements

```text
System requirement affected?
Subsystem requirement affected?
Traceability affected?
```

### Design

```text
Architecture affected?
Interface affected?
Software affected?
Hardware affected?
Data affected?
```

### Verification

```text
Existing test still valid?
New test required?
Regression required?
Analysis required?
Simulation required?
```

## 8. Safety significance classification

Preliminary:

```text
LEVEL-0
No safety/certification impact identified

LEVEL-1
Potential operational impact

LEVEL-2
Safety-related impact

LEVEL-3
Safety-significant impact

LEVEL-4
Potential certification-critical impact
```

These levels are project control categories, not aircraft safety classification or regulatory assurance levels.

## 9. Change workflow

```text
1. Submit Change
2. Assign Change ID
3. Identify affected configuration
4. Perform impact assessment
5. Determine safety impact
6. Determine certification impact
7. Determine required verification
8. Approve / reject
9. Implement
10. Verify
11. Update evidence
12. Update traceability
13. Update configuration
14. Close
```

## 10. Emergency change

Emergency changes may be necessary for operational or safety reasons.

Even then:

```text
Emergency Change
→ Minimum safety assessment
→ Authorized implementation
→ Immediate verification
→ Full retrospective documentation
→ Configuration update
```

Emergency status does not eliminate traceability.

## 11. Defect workflow

```text
Detection
→ Problem Record
→ Containment
→ Classification
→ Impact Assessment
→ Root Cause
→ Corrective Action
→ Verification
→ Regression
→ Evidence
→ Closure
```

## 12. Failed verification

A failed verification result must remain part of the history.

```text
FAIL
→ Problem Record
→ Impact Assessment
→ Corrective Action
→ Re-test
→ New Evidence
```

The failed result must not be deleted merely because a later result passes.

## 13. Safety-critical change gate

Changes affecting:

- Navigation;
- Route validation;
- Dynamic Return;
- Collision Avoidance;
- C2;
- Mission Execution;
- Multi-UAV separation;
- Safety Gate;
- Authorization;
- AI authority;

must receive explicit safety impact assessment before closure.

## 14. Certification-critical change gate

Changes affecting:

- certification basis;
- certification requirements;
- safety requirements;
- verification method;
- accepted evidence;
- configuration baseline;
- certification documentation;

must receive certification impact assessment.

## 15. Traceability update

After approved change:

```text
Source
→ Requirement
→ Design
→ Implementation
→ Verification
→ Evidence
```

must be rechecked for affected records.

## 16. Evidence invalidation

If a change invalidates previously accepted evidence:

```text
Old Evidence
→ INVALIDATED / SUPERSEDED
→ Impact Record
→ New Verification
→ New Evidence
```

Evidence must never silently remain marked valid against an incompatible configuration.

## 17. Configuration update

After implementation and verification:

```text
Configuration Item
→ New Version
→ Change ID
→ Verification Result
→ Evidence
→ Baseline
```

## 18. Approval

Minimum roles to consider:

```text
Change Owner
System Owner
Safety Owner
Verification Owner
Configuration Owner
Certification Owner
Approver
```

Required approval levels depend on impact.

## 19. Change decision

Permitted decisions:

```text
APPROVE
APPROVE WITH CONDITIONS
REJECT
DEFER
REQUEST MORE ANALYSIS
```

## 20. Closure criteria

A change may be closed only when:

```text
Impact Assessment complete
+
Required approval complete
+
Implementation complete
+
Required verification complete
+
Evidence updated
+
Traceability updated
+
Configuration updated
```

For safety/certification-critical changes, additional authority acceptance may be required.

## 21. Problem closure

A problem may be closed only when:

```text
Cause identified or disposition justified
+
Containment complete
+
Corrective action complete
+
Verification complete
+
Evidence updated
+
Affected baselines updated
```

## 22. Change impact on Safety Case

If a change affects a Safety Case claim:

```text
Claim
→ Argument
→ Requirements
→ Hazard
→ Mitigation
→ Verification
→ Evidence
```

must be re-evaluated.

## 23. Change impact on Compliance Matrix

If a change affects a compliance record:

```text
Compliance Record
→ Requirement
→ MoC
→ Evidence
→ Configuration
```

must be reviewed.

## 24. Change impact on Requirements Traceability

If a change affects a requirement:

```text
Requirement
→ Design
→ Implementation
→ Verification
→ Evidence
```

must be rechecked.

## 25. Records retention

Change and problem history is part of project engineering and certification history.

Superseded records remain traceable.

## 26. Current gaps

```text
CHG-GAP-001  Formal change management tool/process not established
CHG-GAP-002  Roles not formally assigned
CHG-GAP-003  Approval thresholds not established
CHG-GAP-004  Safety impact methodology not approved
CHG-GAP-005  Certification impact methodology not approved
CHG-GAP-006  Problem database not established
CHG-GAP-007  Evidence invalidation mechanism not automated
CHG-GAP-008  Configuration/change integration not automated
```

## 27. Next document

Следующий документ:

`INTERFACE_CONTROL_DOCUMENT.md`

Он определит контролируемые интерфейсы между подсистемами авиационной системы и внешними объектами.

## 28. Status

**DRAFT_FOR_AGREEMENT**
