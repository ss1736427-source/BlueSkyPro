---
id: NAVIGATION-BASELINE-RECON-001
type: navigation_baseline_reconciliation
status: working
version: 0.2
parent: AVIATION-SYSTEM-DEFINITION-001
authority: MASTER-REQUIREMENTS-REGISTER-001; AVIATION-SYSTEM-DEFINITION-001; NAVIGATION-STATE-MODEL-001
---

# BlueSky PRO — Navigation Baseline Reconciliation

## 1. Назначение

Единая точка сведения существующей документации Navigation. Документ не создаёт новую независимую базу требований и не заменяет authoritative documents.

Его задача — показать, как уже существующие знания, требования, архитектура, правила, алгоритмы, математические определения, модули, HMI, verification и evidence связаны между собой, и где связь ещё не подтверждена.

## 2. Authoritative hierarchy

```text
OFFICIAL / CONTROLLED SOURCE
        ↓
REGULATORY APPLICABILITY
        ↓
CERTIFICATION BASIS
        ↓
MASTER REQUIREMENTS REGISTER
        ↓
SYSTEM REQUIREMENTS SPECIFICATION
        ↓
AVIATION SYSTEM DEFINITION
        ↓
NAVIGATION STATE MODEL
        ↓
NAVIGATION RULES
        ↓
NAVIGATION ALGORITHM
        ↓
NAVIGATION MATHEMATICS
        ↓
MODULE SPECIFICATION
        ↓
INTERFACE / HMI ALLOCATION
        ↓
VERIFICATION
        ↓
EVIDENCE
```

Важное ограничение: фактический источник требования определяется записью в `MASTER-REQUIREMENTS-REGISTER-001`; отсутствие подтверждённой связи обозначается как `TBD`, `PENDING` или `UNALLOCATED`, а не заменяется предположением.

## 3. Authoritative project documents

| Layer | Document | ID | Role |
|---|---|---|---|
| Requirements | `01_REQUIREMENTS/SYSTEM/MASTER_REQUIREMENTS_REGISTER.md` | `MASTER-REQUIREMENTS-REGISTER-001` | identity of requirements |
| Architecture | `02_ARCHITECTURE/SYSTEM/AVIATION_SYSTEM_DEFINITION.md` | `AVIATION-SYSTEM-DEFINITION-001` | system boundary and functional architecture |
| Navigation State | `03_SYSTEM/NAVIGATION/NAVIGATION_STATE_MODEL.md` | `NAVIGATION-STATE-MODEL-001` | navigation state semantics |
| Rules | `03_SYSTEM/NAVIGATION/NAVIGATION_RULES.md` | `NAVIGATION-RULES-001` | navigation rules |
| Algorithm | `03_SYSTEM/NAVIGATION/NAVIGATION_ALGORITHM.md` | `NAVIGATION-ALGORITHM-001` | logical algorithm |
| Mathematics | `03_SYSTEM/NAVIGATION/NAVIGATION_MATHEMATICAL_SPECIFICATION_001.md` | `NAVIGATION-MATH-SPEC-001` | mathematical layer |
| Verification | `05_VERIFICATION/NAVIGATION/NAVIGATION_VERIFICATION_MODEL.md` | `NAVIGATION-VERIFICATION-MODEL-001` | verification framework |

## 4. Knowledge sources

```text
00_PROJECT/KNOWLEDGE/NAVIGATION_KNOWLEDGE_REVIEW_001.md
        ↓
03_SYSTEM/NAVIGATION/Navigation_Knowledge_Map.md
        ↓
NAVIGATION STATE / RULES / ALGORITHM
```

`00_PROJECT/Самолетовождение_Черный_Кораблин_1973_KNOWLEDGE_BASE.md` является техническим knowledge source. Он не является сам по себе нормативным требованием или certification approval.

## 5. Module allocation

| Module | ID | Parent | Authoritative semantic source | Mathematical source | Verification source | Status |
|---|---|---|---|---|---|---|
| Position | `NAVIGATION-POSITION-MODULE-001` | `NAVIGATION-STATE-MODEL-001` | State Model / Rules / Algorithm | `NAVIGATION-MATH-SPEC-001` | `NAVIGATION-VERIFICATION-MODEL-001` | draft_for_agreement |
| Velocity / Speed | `NAVIGATION-VELOCITY-SPEED-MODULE-001` | `NAVIGATION-STATE-MODEL-001` | State Model / Rules / Algorithm | `NAVIGATION-MATH-SPEC-001` | `NAVIGATION-VERIFICATION-MODEL-001` | draft_for_agreement |
| Heading | `NAVIGATION-HEADING-MODULE-001` | `NAVIGATION-STATE-MODEL-001` | State Model / Rules / Algorithm | `NAVIGATION-MATH-SPEC-001` | `NAVIGATION-VERIFICATION-MODEL-001` | draft_for_agreement |
| Track / Course | `NAVIGATION-TRACK-COURSE-MODULE-001` | `NAVIGATION-STATE-MODEL-001` | State Model / Rules / Algorithm | `NAVIGATION-MATH-SPEC-001` | `NAVIGATION-VERIFICATION-MODEL-001` | draft_for_agreement |
| Wind | `NAVIGATION-WIND-MODULE-001` | `NAVIGATION-STATE-MODEL-001` | State Model / Rules / Algorithm | `NAVIGATION-MATH-SPEC-001` | `NAVIGATION-VERIFICATION-MODEL-001` | draft_for_agreement |

## 6. Module relationship model

```text
                    NAVIGATION STATE
                          │
          ┌───────────────┼───────────────┐
          ↓               ↓               ↓
      POSITION       VELOCITY/SPEED     HEADING
          │               │               │
          └───────────────┼───────────────┘
                          ↓
                   WIND / ENVIRONMENT
                          ↓
                  TRACK / COURSE
                          ↓
              DERIVED NAVIGATION DATA
                          ↓
              ROUTE / DEVIATION / ETA
                          ↓
                 FEASIBILITY / RETURN
```

Это dependency model. Она не означает, что каждый модуль владеет соответствующими решениями. Execution authority остаётся за установленным system authority chain.

## 7. Semantic consistency check

Уже согласованные в State Model и Rules различия должны использоваться всеми модулями без переопределения:

```text
PLANNED ≠ ACTUAL
SIMULATED ≠ ACTUAL
Course ≠ Heading ≠ Track ≠ Bearing
Airspeed ≠ Groundspeed
MEASURED ≠ ESTIMATED ≠ DERIVED ≠ PREDICTED
Navigation State ≠ Execution Authority
```

Контрольное правило: модуль может уточнять применение существующей семантики, но не создавать альтернативное определение.

## 8. Requirement linkage

`MASTER-REQUIREMENTS-REGISTER-001` устанавливает:

```text
ONE REQUIREMENT
→ ONE STABLE ID
→ ONE CONTROLLED WORDING
→ MANY RELATIONSHIPS
```

Candidate family `NAV-REQ-*` существует, но её записи не становятся baseline автоматически.

Для каждого Navigation requirement применяется:

```text
Existing SYS-REQ / SAF-REQ
        ↓
exact text comparison
        ↓
Navigation source/rule/model comparison
        ↓
regulatory applicability
        ↓
KEEP / DERIVED / MERGE / GAP / CONFLICT
        ↓
allocation to module
```

До завершения сравнения конкретный `NAV-REQ-*` считается `CANDIDATE`, а не утверждённым системным требованием.

## 9. Safety linkage

Системная граница подтверждена `AVIATION-SYSTEM-DEFINITION-001`:

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

Navigation является поставщиком состояния и расчётных результатов. Ни один Navigation module не получает execution authority.

## 10. Verification linkage

`NAVIGATION-VERIFICATION-MODEL-001` является verification framework, а не второй requirements database.

Базовые verification scenarios:

```text
NAV-V01 … NAV-V20
```

При сведении используется правило:

```text
существующее покрытие → LINK
нет покрытия → GAP CANDIDATE
изменено требование/алгоритм → IMPACT + REGRESSION REVIEW
```

Конкретные numerical acceptance criteria остаются `TBD`, пока не закрыты соответствующие mathematical specifications.

## 11. Mathematical linkage

`NAVIGATION-MATH-SPEC-001` является единственным текущим специализированным математическим слоем Navigation.

Он покрывает, в частности:

```text
Wind triangle
Course / Heading / Track
WCA
Drift Angle
Groundspeed
Cross Track Error
Along Track Position
ETA
Return Feasibility
Reference Frames
Quality / temporal validity
```

Открытые математические параметры не должны дублироваться в module specifications с другими значениями.

## 12. HMI / Interface linkage

Navigation HMI должен представлять authoritative Navigation State и его quality, а не создавать независимую navigation truth.

Трасса:

```text
Navigation State
→ HMI allocation
→ UI representation
```

Конкретные HMI requirements и interface schemas связываются с соответствующими существующими документами при их фактическом наличии. Несуществующие документы не указываются как authority.

## 13. Certification linkage

Сертификационная связь не выводится только из названия модуля.

Применяется:

```text
OFFICIAL SOURCE
→ CLAUSE
→ APPLICABILITY
→ CERTIFICATION BASIS
→ REQUIREMENT
→ DESIGN
→ VERIFICATION
→ EVIDENCE
```

Если официальный clause mapping для конкретной Navigation функции ещё не установлен:

```text
Certification linkage = PENDING
```

Это не является заявлением о compliance.

## 14. Current reconciliation status

| Area | State | Action |
|---|---|---|
| Navigation document inventory | established | maintain |
| Module IDs / metadata | established for current five modules | verify consistency |
| State semantics | established | no duplicate definitions |
| Rules ↔ State | linked by authority | consistency review |
| Algorithm ↔ Rules | linked by authority | consistency review |
| Mathematics ↔ Algorithm | linked | close TBD parameters later |
| Modules ↔ Mathematics | linked | verify exact allocation |
| Verification ↔ Algorithm | linked | map exact cases |
| Requirements ↔ Modules | not fully allocated | reconcile exact records |
| Regulatory clauses ↔ modules | not fully allocated | do not invent mapping |
| HMI ↔ modules | partial | allocate through HMI documents |
| Evidence ↔ tests | existing framework | map when evidence exists |

## 15. No-duplication rule

```text
IF information already exists
→ reference it

IF information is incomplete
→ update authoritative/owning document

IF information is duplicated
→ consolidate and preserve history

IF information is genuinely absent
→ create controlled new document
```

A module specification is not allowed to become a parallel requirements, mathematics, HMI or verification database.

## 16. Open reconciliation items

1. Exact allocation of existing `SYS-REQ-*` to Navigation functions/modules.
2. Exact allocation of existing `SAF-REQ-*` where Navigation is safety-significant.
3. Clause-level regulatory mapping for applicable Navigation functions.
4. Exact interface document allocation.
5. HMI requirement allocation.
6. Numerical parameters and acceptance tolerances.
7. Source/fusion policy.
8. Reference-frame and altitude conventions.
9. Verification case-to-requirement allocation.
10. Evidence mapping.

## 17. Completion gate for Navigation

Navigation cannot be marked complete merely because all module specification files exist.

Required:

```text
[ ] all existing requirements reviewed
[ ] no duplicate requirement created
[ ] each applicable requirement allocated
[ ] safety allocation checked
[ ] regulatory applicability checked
[ ] state/rules/algorithm/math consistent
[ ] all five current modules reconciled
[ ] interfaces allocated
[ ] HMI allocated
[ ] verification coverage mapped
[ ] evidence path defined
[ ] open engineering parameters resolved or formally accepted
[ ] Master Document Index synchronized
```

## 18. Controlled next operation

Следующий проход выполняется по содержимому, а не по созданию новых файлов:

```text
Position
→ Velocity / Speed
→ Heading
→ Track / Course
→ Wind
        ↓
exact cross-check against
State Model / Rules / Algorithm / Mathematics
        ↓
requirements allocation
        ↓
verification allocation
        ↓
HMI / interface allocation
        ↓
open-item register
        ↓
Navigation completion review
```

**Status: WORKING — RECONCILIATION IN PROGRESS**
