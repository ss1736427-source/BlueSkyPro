---
id: NAVIGATION-SYSREQ-CONTENT-RECON-002
type: navigation_system_requirement_content_reconciliation
status: working
version: 0.1
parent: NAVIGATION-BASELINE-RECON-001
authority: MASTER-REQUIREMENTS-REGISTER-001
---

# BlueSky PRO — Navigation ↔ SYS-REQ Content Reconciliation Pass 002

## 1. Purpose

Провести содержательную сверку существующих `SYS-REQ` с Navigation без создания новых requirement IDs и без подмены отсутствующего controlled wording предположениями.

## 2. Source state verified

`MASTER_REQUIREMENTS_REGISTER.md` подтверждает следующие existing records и их titles:

| ID | Existing title | Navigation relation |
|---|---|---|
| SYS-REQ-081 | UAV Failure Tolerance | direct/partial candidate |
| SYS-REQ-082 | Safe Mission Completion | direct/partial candidate |
| SYS-REQ-085 | Safety-Critical Priority | safety/authority relation |
| SYS-REQ-086 | Graceful Degradation | direct/partial candidate |
| SYS-REQ-091 | Critical Latency | temporal/freshness relation |
| SYS-REQ-093 | Controlled Resource Recovery | recovery relation |

The Master Register explicitly states that existing IDs are preserved and that Navigation candidates require overlap/source analysis before baseline.

## 3. Exact wording availability

На текущем `main` полный controlled requirement text для перечисленных `SYS-REQ-081/082/085/086/091/093` в `MASTER_REQUIREMENTS_REGISTER.md` отсутствует: register содержит ID, title и architecture linkage, но не полный текст требования.

Следовательно, построчная wording-equivalence проверка с `NAV-REQ-*` сейчас **не может быть доказательно завершена** только по Master Register.

Статус wording comparison:

```text
SYS-REQ exact wording → NOT PRESENT IN REGISTER
NAV-REQ-001 exact controlled wording/source → NOT CONFIRMED
NAV-REQ-002..009 individual records → NOT CONFIRMED
```

## 4. Decisions

| Record | Decision | Basis |
|---|---|---|
| SYS-REQ-081 | KEEP | existing controlled identity; Navigation relation remains allocation candidate |
| SYS-REQ-082 | KEEP | existing controlled identity; Navigation relation remains allocation candidate |
| SYS-REQ-085 | KEEP | existing safety/authority identity; do not merge into Navigation |
| SYS-REQ-086 | KEEP | existing controlled identity; Navigation degradation relation remains candidate |
| SYS-REQ-091 | KEEP | existing performance identity; possible Navigation temporal-validity allocation |
| SYS-REQ-093 | KEEP | existing recovery identity; possible Navigation recovery allocation |

No `MERGE` decision is permitted until exact wording is available.

## 5. Navigation candidate status

```text
NAV-REQ-001   CANDIDATE / WORDING-SOURCE PENDING
NAV-REQ-002   NOT INDIVIDUALLY CONFIRMED
NAV-REQ-003   NOT INDIVIDUALLY CONFIRMED
NAV-REQ-004   NOT INDIVIDUALLY CONFIRMED
NAV-REQ-005   NOT INDIVIDUALLY CONFIRMED
NAV-REQ-006   NOT INDIVIDUALLY CONFIRMED
NAV-REQ-007   NOT INDIVIDUALLY CONFIRMED
NAV-REQ-008   NOT INDIVIDUALLY CONFIRMED
NAV-REQ-009   NOT INDIVIDUALLY CONFIRMED
```

## 6. Engineering allocation currently supported

The existing Navigation baseline defines the controlled chain:

```text
Navigation State
→ Rules
→ Algorithm
→ Mathematics
→ Module
→ Verification
```

Current Navigation modules:

```text
Position
Velocity / Speed
Heading
Track / Course
Wind
```

The semantic controls `Course ≠ Heading ≠ Track` and `Airspeed ≠ Groundspeed` remain authoritative within the Navigation engineering layer.

These engineering documents establish implementation structure; they do **not** by themselves prove that a candidate `NAV-REQ-*` is an independent system requirement.

## 7. Required closure evidence

For each `SYS-REQ-081/082/085/086/091/093` the next controlled step is to obtain:

```text
Exact Requirement Text
→ Source / Basis
→ Navigation applicability
→ Exact allocation
→ Architecture Decision
→ Design element
→ Verification Case
→ Evidence
→ Configuration
```

Until that chain is populated, status remains open.

## 8. No-change rule

```text
DO NOT renumber existing SYS-REQ
DO NOT create NAV-REQ-002..009 from module specifications
DO NOT declare MERGE without exact wording comparison
DO NOT claim certification compliance from engineering linkage alone
```

## 9. Result

**CONTENT RECONCILIATION: PARTIALLY CLOSED**

The six identified `SYS-REQ` records are retained as existing requirements. Their possible Navigation allocation is recorded as a candidate relationship only. Exact controlled wording remains the gating item for final `KEEP / DERIVED / MERGE / GAP / CONFLICT` determination.

## 10. Next controlled operation

Obtain the actual requirement-text source for the six existing `SYS-REQ` records and perform the exact wording comparison before any baseline change.
