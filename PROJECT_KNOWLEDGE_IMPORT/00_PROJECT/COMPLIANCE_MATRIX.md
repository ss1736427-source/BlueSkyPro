---
id: COMPLIANCE-MATRIX-001
type: compliance_matrix
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
certification_object: CERTIFICATION-OBJECT-AND-SCOPE-001
certification_basis: CERTIFICATION-BASIS-001
requirement_authority: MASTER-REQUIREMENTS-REGISTER-001
---

# Compliance Matrix

## 1. Назначение

Единая матрица подтверждения соответствия для конкретного заявленного certification scope BlueSky.

Матрица связывает:

```text
Official Source / Clause
→ Certification Basis
→ Certification Requirement
→ System Requirement
→ Compliance Method
→ Verification
→ Evidence
→ Configuration
```

Матрица не является второй базой требований.

## 2. Certification object

Рабочая модель объекта определена в:

`CERTIFICATION_OBJECT_AND_SCOPE.md`

```text
БАС нового типа и/или сертифицируемый элемент
        ↓
BlueSky Aviation System
        ↓
конкретная configuration
```

Окончательный scope подлежит установлению для заявляемой конфигурации.

## 3. Certification profiles

| Profile | Scope | Status | Compliance rule |
|---|---|---|---|
| PROFILE-A | БАС до 30 кг включительно | Initial strategic profile | Applicability определяется отдельно |
| PROFILE-B | БАС свыше 30 кг | Future expansion | Separate certification scope / basis |
| PROFILE-C | Иные применимые конфигурации | Future | Separate applicability assessment |

Российское регулирование требует учитывать применимый режим отдельно: ст. 8 ВК РФ исключает из обязательной сертификации БАС/их элементы, включающие БВС с MTOM 30 кг и менее, тогда как ст. 37 устанавливает обязательную сертификацию БАС нового типа и/или их элементов в предусмотренных законом случаях. citeturn0search2turn0search0

Поэтому `PROFILE-A` не следует описывать как «сертифицированный профиль» без отдельного правового основания.

## 4. Compliance status model

Используются:

```text
NOT_STARTED
APPLICABILITY_PENDING
SOURCE_PENDING
MAPPED
PARTIAL
COMPLIANCE_ARGUMENT_DEFINED
VERIFICATION_PENDING
EVIDENCE_PENDING
COMPLIANT_PENDING_REVIEW
COMPLIANT
NOT_APPLICABLE
CONFLICT
DEFERRED
```

`COMPLIANT` допускается только после установленного compliance argument и необходимого evidence/review.

## 5. Source authority

Для каждой строки обязательны:

```text
Source ID
Official title
Revision / date
Clause
Effective status
Applicability
```

Официальная нормативная база должна подтверждаться по действующим источникам.

Для certification basis ВК РФ прямо определяет его как совокупность требований, необходимых для безопасной эксплуатации БАС/их элементов, а порядок разработки и утверждения устанавливается федеральными авиационными правилами. citeturn0search0

## 6. Core matrix

| CB ID | Source / Clause | Applicability | Certification Requirement | System Requirement | Method | Verification | Evidence | Profile | Status |
|---|---|---|---|---|---|---|---|---|---|
| CB-001 | TBD | TBD | Mandatory certification applicability | TBD | Legal/regulatory review | TBD | TBD | A/B/C | APPLICABILITY_PENDING |
| CB-002 | TBD | TBD | Certification basis applicability | TBD | Analysis | TBD | TBD | A/B/C | SOURCE_PENDING |
| CB-003 | TBD | TBD | Type / configuration conformity | TBD | Inspection / analysis | TBD | TBD | B | APPLICABILITY_PENDING |
| CB-004 | TBD | TBD | Flightworthiness-related requirement | TBD | Analysis / test | TBD | TBD | B | SOURCE_PENDING |

Эти строки являются **рабочими placeholders**, а не нормативными утверждениями.

## 7. Existing system requirement linkage

Системные требования берутся только из:

`MASTER_REQUIREMENTS_REGISTER.md`

Примеры:

| SYS-REQ | Certification relevance | Architecture | Compliance status |
|---|---|---|---|
| SYS-REQ-080 | TBD | ARCH-DEC-008, 012, 039 | APPLICABILITY_PENDING |
| SYS-REQ-081 | Safety / certification relevance to assess | ARCH-DEC-017, 027, 040 | APPLICABILITY_PENDING |
| SYS-REQ-082 | Safety / certification relevance to assess | ARCH-DEC-016, 017, 038 | APPLICABILITY_PENDING |
| SYS-REQ-084 | System requirement | ARCH-DEC-036, 037 | APPLICABILITY_PENDING |
| SYS-REQ-085 | Safety-critical | ARCH-DEC-007, 016, 036 | APPLICABILITY_PENDING |
| SYS-REQ-086 | Safety / recovery | ARCH-DEC-017, 023, 039 | APPLICABILITY_PENDING |
| SYS-REQ-087 | AI / resource isolation | ARCH-DEC-022, 023, 035 | APPLICABILITY_PENDING |
| SYS-REQ-088 | Safe learning | ARCH-DEC-011, 025, 026 | APPLICABILITY_PENDING |
| SYS-REQ-089 | Resource monitoring | ARCH-DEC-022, 024, 032 | APPLICABILITY_PENDING |
| SYS-REQ-090 | Resource protection | ARCH-DEC-022, 036 | APPLICABILITY_PENDING |
| SYS-REQ-091 | Performance / latency | ARCH-DEC-022 | APPLICABILITY_PENDING |
| SYS-REQ-092 | Recovery | ARCH-DEC-027, 040 | APPLICABILITY_PENDING |
| SYS-REQ-093 | Recovery | ARCH-DEC-017, 036 | APPLICABILITY_PENDING |

These rows do not assert certification applicability until source mapping is completed.

## 8. Safety compliance

Safety chain:

```text
HAZARD
→ SAFETY OBJECTIVE
→ SAFETY REQUIREMENT
→ SYSTEM REQUIREMENT
→ ARCHITECTURE
→ DESIGN
→ VERIFICATION
→ EVIDENCE
```

Existing architecture authority:

```text
MISSION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ OPERATOR APPROVAL
→ EXECUTION
```

Candidate requirements:

```text
SAF-REQ-019
SAF-REQ-020
AUTH-REQ-001
AUTH-REQ-002
AI-REQ-002
```

remain subject to consolidation before being treated as independent certification requirements.

## 9. Navigation compliance

Candidate:

```text
NAV-REQ-001..009
```

Engineering source chain:

```text
Original technical source
→ Knowledge Base
→ Navigation Knowledge Map
→ Engineering Rule
→ Requirement
```

Regulatory chain:

```text
Official Russian Source
→ Clause
→ Applicability
→ Certification Requirement
```

These chains must remain separate.

For navigation compliance, each applicable requirement must identify, where relevant:

```text
Reference frame
Coordinate system
Units
Timestamp
Validity
Accuracy / quality
Source
Freshness
Verification method
```

## 10. Route / waypoint / mission

Candidate families:

```text
RTE-REQ-001..004
WP-REQ-001..003
MIS-REQ-001..003
```

Status:

```text
APPLICABILITY_PENDING
```

No certification claim until their source and scope are established.

## 11. Dynamic Return

Candidates:

```text
RET-REQ-001..004
```

Compliance chain:

```text
Validated State
→ Return Feasibility
→ Return Route
→ Safety Validation
→ Authorization / Operator Approval
→ Execution
→ Verification
```

Quantitative thresholds are `TBD` until established by approved requirements/analysis.

## 12. Collision avoidance

Candidates:

```text
COL-REQ-001..004
```

Compliance chain:

```text
Track
→ Prediction
→ Conflict Detection
→ Separation Assessment
→ Avoidance Proposal
→ Authority
→ Execution
→ Verification
```

Exact separation values remain `TBD`.

## 13. C2 / Multi-UAV

Candidates:

```text
C2-REQ-001..003
MUL-REQ-001..003
```

Required compliance dimensions:

```text
Communication integrity
Loss-of-link behaviour
Authority
State consistency
Failure containment
Interface conformity
Verification
```

Status:

```text
APPLICABILITY_PENDING
```

## 14. Configuration compliance

Each compliance claim is bound to:

```text
Aircraft
BlueSky version
Software version
Hardware
Interfaces
Parameters
Data
Configuration baseline
Verification evidence
```

A compliance result shall not be assumed portable to another configuration.

## 15. Profile A — ≤30 kg

The profile is strategically important for initial market entry.

However, the matrix must distinguish:

```text
No mandatory certification under a particular provision
≠
No regulatory obligations
≠
No product requirements
≠
No evidence
```

The current Russian framework also contains separate mandatory-requirement material concerning state accounting of civil unmanned aircraft from 0.15 kg to 30 kg, showing that the absence of the relevant certification obligation does not eliminate the need for a broader regulatory applicability review. citeturn0search9

Therefore PROFILE-A requires its own:

```text
Regulatory applicability matrix
Operational requirements
Equipment requirements
Registration / accounting review
C2 / communications review
Safety review
Product evidence
```

## 16. Profile B — >30 kg

For >30 kg configurations:

```text
Certification applicability
→ Certification basis
→ Compliance requirements
→ Verification
→ Evidence
→ Type / conformity pathway
```

The Russian framework expressly provides certification of BАС new type and/or elements and defines the certification basis as the set of requirements necessary for safe operation. citeturn0search0

Rosaviatsiya also publishes a list of regulatory acts containing mandatory requirements for issuance, suspension and annulment of the airworthiness certificate for an unmanned civil aircraft with MTOM over 30 kg. citeturn0search6

Therefore PROFILE-B must have a dedicated clause-level regulatory mapping.

## 17. Common core vs configuration-specific compliance

### Common candidates

```text
Safety architecture
Configuration management
Software development controls
Verification process
Data integrity
Common interfaces
Common functional behaviour
```

### Configuration-specific candidates

```text
Flight dynamics
Aircraft performance
Mass properties
Energy/endurance
Sensors
Actuators
Navigation equipment
C2 equipment
Environmental limitations
Operational limitations
Aircraft-specific safety
```

A common compliance argument is valid only where applicability and technical equivalence are demonstrated.

## 18. Verification mapping

Compliance methods:

```text
ANALYSIS
INSPECTION
TEST
SIMULATION
DEMONSTRATION
REVIEW_OF_DESIGN
DOCUMENT_REVIEW
```

Target:

```text
Certification Requirement
→ System Requirement
→ Compliance Method
→ Verification Case
→ Result
→ Evidence
```

Existing `TEST-*` records are reused when coverage is equivalent.

## 19. Evidence mapping

Each compliance row should eventually contain:

```text
Evidence ID
Evidence type
Configuration
Execution date
Result
Reviewer
Baseline
```

No evidence means no final `COMPLIANT` status.

## 20. Regulatory source register relationship

Source records are maintained in:

`REGULATORY_SOURCE_REGISTER.md`

This matrix references source IDs rather than duplicating source metadata wherever practical.

## 21. Certification requirements baseline

Approved certification requirements will be represented in:

`CERTIFICATION_REQUIREMENTS_BASELINE.md`

Workflow:

```text
Source
→ Applicability
→ Certification Basis
→ Compliance Matrix
→ Certification Requirement
→ System Requirement
```

## 22. Change impact

Any change to:

```text
Source
Certification Basis
Certification Requirement
System Requirement
Configuration
Architecture
Design
Verification
Evidence
```

requires impact assessment.

Minimum impact:

```text
Certification
Safety
Requirements
Architecture
Verification
Evidence
Configuration
```

## 23. Compliance decision rules

### COMPLIANT

Only if:

```text
Source verified
Applicability verified
Requirement established
Compliance method established
Verification/evidence complete
Configuration identified
Review complete
```

### PARTIAL

Some evidence/coverage exists, but a required element is missing.

### APPLICABILITY_PENDING

Legal/regulatory applicability has not been established.

### SOURCE_PENDING

Official source/clause has not been verified.

### NOT_APPLICABLE

Applicability has been formally determined to be false for the specific scope/configuration.

### CONFLICT

Two authoritative requirements or interpretations require resolution.

## 24. Current matrix state

```text
Certification Object: DEFINED
Product Family: DEFINED
Profile A ≤30 kg: INITIAL STRATEGIC PROFILE
Profile B >30 kg: FUTURE EXPANSION PROFILE

Official clause mapping: IN PROGRESS
Applicability determination: IN PROGRESS
Certification requirements baseline: IN PROGRESS
Compliance evidence mapping: PENDING
Authority-agreed certification scope: PENDING
```

## 25. Immediate next operation

Now that the object and basis are aligned, the next step is **external regulatory source reconciliation**, not another template.

Required work:

```text
1. Build official source register.
2. Verify current status of each source.
3. Determine applicability to PROFILE-A.
4. Determine applicability to PROFILE-B.
5. Identify exact certification procedure.
6. Map clauses to certification requirements.
7. Map requirements to Master Requirements Register.
8. Define compliance methods.
9. Identify evidence.
10. Record gaps.
```

## 26. Status

**DRAFT_FOR_AGREEMENT**



