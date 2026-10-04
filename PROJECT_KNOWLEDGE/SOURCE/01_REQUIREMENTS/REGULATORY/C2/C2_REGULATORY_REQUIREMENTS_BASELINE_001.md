---
id: C2-REG-BASELINE-001
type: c2_regulatory_requirements_baseline
status: working_draft
system: BlueSky PRO
source: REGULATORY_SOURCE_REGISTER.md
basis: REGULATORY_CLAUSE_MAPPING_001.md
---

# BLUE SKY PRO — C2 REGULATORY REQUIREMENTS BASELINE 001

## 1. Назначение

Документ переводит установленную границу C2 из regulatory mapping в рабочие требования к системе BlueSky PRO.

Это **не утверждённый certification basis** и не заменяет текст нормативного акта.

## 2. Нормативная основа

Приказ Минтранса России от 24.04.2025 №142 устанавливает ФАП по организации и обеспечению функционирования линий управления БАС и контроля БАС; приказ действует с 01.03.2026 по 01.03.2032. citeturn0search2turn0search5

Правила предусматривают два способа организации C2: прямой канал между станцией внешнего пилота и БВС, включая предусмотренные ретрансляторы, либо через инфраструктуру поставщика услуг линии C2. citeturn0search3turn0search23

## 3. Requirement allocation

| ID | Requirement class | BlueSky allocation | Status | Verification |
|---|---|---|---|---|
| C2-REG-001 | C2 mode identification | DIRECT | REQUIRED FOR DESIGN | inspection / test |
| C2-REG-002 | C2 state monitoring | DIRECT | REQUIRED FOR DESIGN | test |
| C2-REG-003 | Loss/degradation state distinction | DIRECT | REQUIRED FOR DESIGN | test / safety analysis |
| C2-REG-004 | Safe response to C2 degradation/loss | DIRECT + SAFETY INTERFACE | REQUIRED FOR ALLOCATION | safety analysis / test |
| C2-REG-005 | C2 provider-side information exchange | INTERFACE | CONDITIONAL | interface test |
| C2-REG-006 | C2 event recording | DIRECT | REQUIRED FOR DESIGN | inspection / test |
| C2-REG-007 | Unified event time model | DIRECT | REQUIRED FOR DESIGN | inspection / test |
| C2-REG-008 | Information-security controls at C2 boundary | DIRECT + INTERFACE | CONDITIONAL / OPEN | security verification |
| C2-REG-009 | Decision time for C2 loss | SAFETY / OPERATIONAL | PARAMETER TO BE DERIVED | analysis / test |
| C2-REG-010 | C2 service availability/quality constraints | INTERFACE | CONDITIONAL | interface verification |

## 4. DIRECT-C2 / PROVIDER-C2

### DIRECT-C2

BlueSky architecture shall explicitly identify the direct C2 configuration and the interfaces through which the external pilot station communicates with the aircraft.

### PROVIDER-C2

BlueSky architecture shall explicitly identify the provider boundary. BlueSky shall not assume provider-side functions, infrastructure, spectrum ownership or service-provider obligations unless those functions are expressly included in the BlueSky certification and product scope.

## 5. C2 state model

The system requirements shall distinguish at minimum:

```text
AVAILABLE
DEGRADED
LOST
RECOVERING
RECOVERED
```

The exact numerical thresholds and transition criteria are **not fixed by this baseline**; they must be derived from the applicable operational requirements, aircraft/system characteristics and safety analysis.

## 6. Loss-of-C2 decision

The regulatory mapping identifies the requirement that the duration for a provider's decision on loss of the C2 line must correspond to the operational requirements of the UAS and the airspace class. citeturn0search3

Therefore BlueSky shall contain a controlled parameter for the applicable loss-of-C2 decision logic and shall trace that parameter to:

```text
Operational Requirements
→ Safety Requirements
→ C2 Configuration
→ Flight Logic
→ Verification
→ Evidence
```

The baseline does **not** invent a universal timeout value.

## 7. C2 event traceability

For each C2 state transition relevant to safety or operation, the system design shall define:

- event identifier;
- timestamp;
- aircraft/system identifier;
- current C2 mode;
- previous and new C2 state;
- relevant measured quality/availability parameters;
- automatic action initiated by BlueSky;
- operator notification;
- recovery state, if applicable.

## 8. Interface boundary

The ICD shall separately define:

1. BlueSky-owned functions;
2. aircraft-side interface;
3. external pilot station interface;
4. provider-C2 interface, where applicable;
5. provider-owned functions;
6. external network/infrastructure dependencies.

## 9. Verification strategy

Regulatory-derived C2 requirements shall be verified through an appropriate combination of:

- requirements review;
- architecture/design inspection;
- interface verification;
- functional testing;
- failure/degradation testing;
- safety analysis;
- evidence review.

No requirement shall receive `COMPLIANT` status merely because a corresponding software feature exists; compliance requires defined acceptance criteria and evidence.

## 10. Traceability targets

```text
C2-REG-xxx
    ↓
MASTER_REQUIREMENTS_REGISTER
    ↓
SYSTEM_REQUIREMENTS_SPECIFICATION
    ↓
SYSTEM_ARCHITECTURE_BASELINE
    ↓
INTERFACE_CONTROL_DOCUMENT
    ↓
SAFETY_REQUIREMENTS_BASELINE / HAZARD_LOG
    ↓
VERIFICATION_PLAN
    ↓
VERIFICATION_EVIDENCE_INDEX
```

## 11. Open items

- exact clause-level allocation from the complete text of №142;
- final C2 operational parameters;
- responsibility boundary when PROVIDER-C2 is used;
- security requirements applicable to the selected C2 implementation;
- aircraft-specific loss-of-C2 behavior and safety allocation.

## 12. Status

```text
WORKING_DRAFT
No final certification claim
No final applicability claim
Numerical thresholds not invented
Regulatory source remains authoritative
```
