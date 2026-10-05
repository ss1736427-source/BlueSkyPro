---
id: DOCUMENT-CONTROL-AND-BASELINE-PROCESS-001
type: document_control_and_baseline_process
status: draft_for_agreement
jurisdiction: Russian Federation
system: aviation_system
---

# Document Control and Baseline Process

## 1. Назначение

Установить постоянный процесс управления документацией авиационной системы BlueSky, чтобы новые документы, требования, решения, evidence и изменения встраивались в существующую систему без повторной полной пересборки.

Процесс применяется к:

```text
Requirements
Safety
Architecture
Design
Interfaces
Verification
Evidence
Configuration
Certification
Knowledge
External Sources
```

## 2. Основной принцип

Документ не создаёт автоматически новую «истину».

Каждый новый controlled document должен иметь:

```text
Owner
Type
Unique ID
Status
Source / Basis
Parent / Related Records
Configuration
Change History
Review Status
```

## 3. Single source of truth

Для каждого типа информации определяется authoritative record.

Принцип:

```text
SOURCE
→ CONTROLLED RECORD
→ DERIVED DOCUMENTS
```

Производные документы не должны независимо переопределять authoritative record.

Если два документа противоречат друг другу, противоречие регистрируется и разрешается через Change / Review Process.

## 4. Knowledge hierarchy

Для инженерных знаний используется:

```text
GENERAL PROJECT KNOWLEDGE
        ↓
SPECIALIZED SOURCE RECORD
        ↓
ORIGINAL SOURCE
        ↓
EXTERNAL AUTHORITATIVE SOURCE
        ↓
COMPARISON
        ↓
CONTRADICTION / AGREEMENT
        ↓
AGREED DECISION
        ↓
CONTROLLED BASELINE
        ↓
ACCUMULATION
```

При этом внешний нормативный источник имеет отдельный статус и не должен смешиваться с проектным engineering judgement.

## 5. Source classes

Каждый source относится к классу:

```text
SRC-REG   Regulatory / official normative source
SRC-CERT  Certification / authority source
SRC-STD   Standard / industry standard
SRC-EXT   External technical source
SRC-ORG   Original technical source
SRC-KB    Project knowledge base
SRC-DES   Project design decision
SRC-REQ   Project requirement
SRC-TEST  Verification evidence
```

Статус источника должен быть виден пользователю.

## 6. Document classes

```text
DOC-REG   Regulatory source register
DOC-CERT  Certification
DOC-REQ   Requirements
DOC-SAF   Safety
DOC-ARCH  Architecture
DOC-DES   Design
DOC-IF    Interface
DOC-VER   Verification
DOC-EVD   Evidence
DOC-CFG   Configuration
DOC-CHG   Change / Problem
DOC-KB    Knowledge
DOC-OPS   Operational
```

## 7. Document lifecycle

```text
DRAFT
→ UNDER REVIEW
→ AGREED
→ BASELINED
→ SUPERSEDED
→ ARCHIVED
```

Не следует использовать `APPROVED` как эквивалент authority certification approval.

Если требуется regulatory/authority approval, это отдельный статус/record.

## 8. Unique identification

Каждая controlled record получает уникальный ID.

Примеры:

```text
SYS-REQ-xxx
SAF-REQ-xxx
HAZ-xxx
ARCH-DEC-xxx
IF-xxx
V-xxx
EVD-xxx
CHG-xxx
PRB-xxx
SRC-xxx
DOC-xxx
BL-xxx
```

ID не переиспользуется после удаления или supersession записи.

## 9. Naming convention

Имена файлов:

```text
DESCRIPTIVE_NAME.md
```

Правила:

- одно понятное имя;
- без случайных суффиксов;
- без `(1)`, `(2)`, `final`, `new`, `new2`;
- версия не кодируется в имени, если версия хранится в front matter / baseline;
- переименование controlled document проходит change process.

## 10. Document front matter

Controlled Markdown document должен по возможности содержать:

```yaml
---
id: UNIQUE-ID
type: DOCUMENT-TYPE
status: DRAFT
owner: ROLE-OR-NAME
version: 0.1
baseline: TBD
source: TBD
parent: TBD
---
```

Если поле неизвестно, используется:

```text
TBD
```

а не выдуманное значение.

## 11. Requirement lifecycle

```text
IDENTIFIED
→ DRAFT
→ REVIEWED
→ AGREED
→ BASELINED
→ VERIFIED
→ SUPERSEDED
```

Для certification requirement дополнительно:

```text
SOURCE
→ APPLICABILITY
→ ALLOCATION
→ COMPLIANCE METHOD
→ EVIDENCE
```

## 12. Requirement creation rule

Новый requirement создаётся только после проверки:

```text
Does an equivalent requirement already exist?
Does a source requirement already exist?
Does a safety requirement already cover it?
Does an architecture/design requirement already cover it?
```

Если эквивалент существует:

```text
LINK
```

а не создаётся второй requirement.

## 13. Requirement change rule

Изменение requirement требует:

```text
CHG
→ Requirement Impact
→ Safety Impact
→ Architecture Impact
→ Design Impact
→ Verification Impact
→ Evidence Impact
→ Baseline Impact
```

## 14. Hazard lifecycle

```text
IDENTIFIED
→ ANALYSIS
→ MITIGATION
→ VERIFICATION
→ RESIDUAL RISK
→ ACCEPTANCE
→ CLOSED
```

Hazard ID остаётся стабильным.

## 15. Safety Case lifecycle

Safety Case claim должен ссылаться на:

```text
Hazard
Safety Objective
Safety Requirement
Design Mitigation
Verification
Evidence
Configuration
```

Изменение любого существенного элемента вызывает impact assessment.

## 16. Architecture / Design rule

Architecture and design documents являются derived engineering records, если для конкретного решения существует authoritative architecture decision.

При конфликте:

```text
ARCHITECTURE DECISION
→ IMPACT REVIEW
→ DESIGN UPDATE
```

Нельзя исправлять один design document так, чтобы противоречие просто исчезло без записи изменения.

## 17. Interface control

Interface changes проходят:

```text
Interface Change
→ Impact Assessment
→ Compatibility Check
→ Safety Impact
→ Verification Impact
→ Evidence Impact
→ Baseline Update
```

## 18. Verification rule

Новый verification case должен быть связан минимум с:

```text
Requirement
or
Safety Objective / Hazard
or
Architecture / Design claim
```

Evidence должно быть связано с verification case.

## 19. Evidence rule

Evidence:

```text
Evidence ID
→ Verification Case
→ Requirement
→ Configuration
```

Если evidence superseded:

```text
OLD EVIDENCE
→ SUPERSEDED
→ NEW EVIDENCE
```

Старое evidence не удаляется из controlled history.

## 20. Configuration rule

Certification-relevant records должны быть связаны с baseline.

Минимальная цепочка:

```text
Document
→ Version
→ Change
→ Review
→ Baseline
```

Для verification:

```text
Test
→ Software Build
→ Data
→ Environment
→ Configuration
```

## 21. Change trigger

Change process запускается при:

```text
New requirement
Requirement change
Safety change
Architecture change
Interface change
Design change
Software change
Hardware change
Data change
Verification method change
Evidence invalidation
Certification source change
External regulatory change
```

## 22. Contradiction handling

При обнаружении противоречия:

```text
1. DO NOT silently overwrite.
2. Identify both records.
3. Identify source authority.
4. Compare context and scope.
5. Determine whether contradiction is real.
6. Create CHG / Problem record where required.
7. Record agreed resolution.
8. Update authoritative record.
9. Update affected derived records.
10. Re-run traceability.
11. Re-run verification where required.
12. Update baseline.
```

## 23. External source update

При изменении внешнего authoritative source:

```text
External Source Change
→ Source Register Update
→ Applicability Review
→ Certification Impact
→ Requirements Impact
→ Safety Impact
→ Compliance Matrix Update
→ Verification Impact
→ Evidence Impact
```

Не требуется пересобирать всю документацию, если impact analysis показывает отсутствие влияния.

## 24. New document onboarding

Каждый новый документ проходит короткую процедуру:

```text
CREATE
→ CLASSIFY
→ ASSIGN ID
→ IDENTIFY SOURCE/BASIS
→ CHECK DUPLICATES
→ LINK TO EXISTING RECORDS
→ IMPACT CHECK
→ REVIEW
→ BASELINE IF REQUIRED
```

## 25. Duplicate prevention

Перед созданием файла:

```text
1. Search by exact filename.
2. Search by document type.
3. Search by title.
4. Search by unique requirement/topic.
5. Search existing IDs.
6. Check Knowledge Map.
```

Если эквивалент найден:

```text
UPDATE EXISTING RECORD
```

а не создавать новый.

## 26. Orphan prevention

Не допускаются без обоснования:

```text
Requirement without source/basis
Requirement without owner
Requirement without verification method
Evidence without verification case
Verification without requirement/claim
Hazard without safety treatment
Design without requirement allocation
Certification claim without evidence plan
```

## 27. Baseline creation

Baseline создаётся после:

```text
Scope defined
+
Records identified
+
Dependencies checked
+
Review completed
+
Open conflicts dispositioned
+
Traceability checked
+
Configuration identified
+
Approval/Agreement recorded
```

## 28. Baseline update

Baseline update не означает пересоздание всего проекта.

```text
Changed Record
→ Impacted Records
→ Required Updates
→ Verification
→ New Baseline Revision
```

Незатронутые документы не переписываются.

## 29. Impact levels

Рабочая классификация:

```text
I0 — Administrative
I1 — Local / no safety effect identified
I2 — Cross-document
I3 — Safety-related
I4 — Certification-significant
```

Это внутренний project control classification, а не regulatory safety classification.

## 30. Review levels

```text
R0 — Author self-check
R1 — Peer review
R2 — Domain review
R3 — Safety review
R4 — Certification review
R5 — Authority / external approval where applicable
```

Конкретный review level определяется impact.

## 31. Master indexes

Должны существовать controlled indexes:

```text
DOCUMENT_INDEX
SOURCE_REGISTER
REQUIREMENTS_REGISTER
HAZARD_LOG
TRACEABILITY_MATRIX
VERIFICATION_EVIDENCE_INDEX
CONFIGURATION_BASELINE
CHANGE_REGISTER
```

Эти indexes являются навигационным слоем, а не заменой первичных records.

## 32. Navigation Knowledge Map

Navigation knowledge follows:

```text
Original Source
→ Specialized Knowledge Base
→ Navigation Knowledge Map
→ Engineering Rule
→ Requirement
→ Design
→ Verification
```

Источник:

`Самолетовождение_Черный_Кораблин_1973_KNOWLEDGE_BASE.md`

не является нормативным источником и не заменяет applicable Russian regulatory requirements.

## 33. Certification documentation hierarchy

```text
REGULATORY SOURCES
        ↓
CERTIFICATION BASIS
        ↓
COMPLIANCE MATRIX
        ↓
CERTIFICATION REQUIREMENTS
        ↓
SYSTEM REQUIREMENTS
        ↓
SAFETY / HAZARDS
        ↓
ARCHITECTURE
        ↓
DESIGN
        ↓
VERIFICATION
        ↓
EVIDENCE
        ↓
CONFIGURATION / CHANGE HISTORY
```

## 34. Obsidian rule

Obsidian является представлением/навигационным слоем controlled project knowledge.

Не должно быть двух независимых «источников истины»:

```text
Filesystem / Controlled Repository
        ↓
Obsidian Navigation / Links
```

Если Obsidian содержит derived notes, они должны иметь ссылку на authoritative record.

## 35. Certification claim rule

Нельзя писать:

```text
CERTIFIED
COMPLIANT
APPROVED BY AUTHORITY
```

если соответствующий status/evidence/authority record отсутствует.

Допустимые рабочие статусы:

```text
DRAFT
UNDER REVIEW
AGREED
BASELINED
VERIFICATION PENDING
VERIFIED
SUBMITTED
ACCEPTED
```

`ACCEPTED` должен иметь определённый authority/context.

## 36. Automation opportunities

Процесс должен постепенно автоматизироваться.

Кандидаты:

```text
Duplicate filename detection
Duplicate ID detection
Broken link detection
Orphan requirement detection
Orphan evidence detection
Missing source detection
Missing verification detection
Baseline consistency check
Configuration consistency check
Change impact report
Traceability completeness report
```

Автоматизация не заменяет engineering judgement.

## 37. Release gate

Перед baseline release выполнять единый check:

```text
[ ] Unique IDs
[ ] No unintended duplicates
[ ] Source identified
[ ] Owner identified
[ ] Status identified
[ ] Traceability complete
[ ] Safety impact checked
[ ] Certification impact checked
[ ] Verification impact checked
[ ] Configuration identified
[ ] Conflicts resolved
[ ] Required review completed
[ ] Obsidian links updated
```

## 38. Master change rule

**Не переделывать всё при каждом новом документе.**

Вместо этого:

```text
NEW RECORD
→ LOCAL IMPACT ANALYSIS
→ UPDATE ONLY AFFECTED RECORDS
→ TRACEABILITY CHECK
→ BASELINE REVISION
```

Полная пересборка требуется только если impact analysis показывает системное изменение baseline.

## 39. Current baseline

После принятия этого процесса формируется:

```text
DOCUMENT-CONTROL-BASELINE-0.1
```

Состав:

```text
Document Control Process
Source Register
Document Index
Requirements Register
Hazard Log
Traceability
Verification Evidence Index
Configuration Baseline
Change / Problem Register
```

## 40. Current gaps

```text
DCP-GAP-001  Master Document Index not yet established
DCP-GAP-002  Master Requirements Register not yet established
DCP-GAP-003  Master Change Register not yet established
DCP-GAP-004  Existing requirement IDs need consolidation
DCP-GAP-005  Existing ARCH-DEC records need linkage
DCP-GAP-006  Existing TEST records need linkage
DCP-GAP-007  Existing navigation knowledge records need linkage
DCP-GAP-008  Official source clause verification pending
DCP-GAP-009  Obsidian navigation layer not fully linked
DCP-GAP-010  Automated consistency checks not established
```

## 41. Mandatory rule from this point

После принятия процесса:

```text
NO BLIND DOCUMENT CREATION
NO DUPLICATE REQUIREMENTS
NO SILENT CONTRADICTION RESOLUTION
NO UNTRACEABLE CERTIFICATION CLAIMS
NO UNCONTROLLED BASELINE CHANGES
```

## 42. Status

**DRAFT_FOR_AGREEMENT**



