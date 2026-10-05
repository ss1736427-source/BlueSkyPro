---
type: traceability_reconciliation
status: draft
scope: NAVIGATION
pass: 003
---

# Navigation ↔ SYS-REQ Content Reconciliation — Pass 003

## Purpose

Фактическая сверка Navigation baseline/capability с существующими SYS-REQ-081, 082, 085, 086, 091 и 093.

Правило: существующие SYS-REQ не переименовываются и не объединяются автоматически. NAV-REQ-001..009 не переводятся в baseline по результатам одной только этой сверки.

## Navigation baseline / capability used for comparison

Navigation capability CAP-001:
- определяет и поддерживает operational trajectory;
- принимает position, velocity, heading, altitude, mission route, waypoints, spatial/environmental data и UAV state;
- выдаёт navigation state, estimated position, estimated motion state, route-relative state и navigation validity;
- не должен сообщать valid navigation state при недостаточной integrity/freshness входных данных;
- safety-critical navigation failures должны быть видимы Safety и Readiness.

CRB-NAV baseline:
- CRB-NAV-001 Navigation State;
- CRB-NAV-002 Navigation data quality;
- CRB-NAV-003 Reference frame;
- CRB-NAV-004 Course / Heading / Track distinction;
- CRB-NAV-005 Navigation deviation.

## Reconciliation matrix

| SYS-REQ | Existing controlled wording | Navigation relation | Verdict | Action |
|---|---|---|---|---|
| SYS-REQ-081 | UAV failure must not automatically cause mission loss when remaining resources can continue or safely complete; system assesses failure impact; critical tasks may be transferred to a reserve executor. | Directly affects surviving/reassigned UAV navigation needed for continuation/safe completion. Existing capability traceability explicitly identifies CAP-001 Navigation as SUPPORTING. | PARTIAL / SUPPORTING | Keep SYS-REQ-081. Link Navigation as supporting allocation; do not merge into NAV-REQ. |
| SYS-REQ-082 | If full mission completion becomes impossible, system shall ensure safe mission completion; scenario considers UAV state, task criticality, resources, energy, communications and risks; safe return/landing/emergency behaviour are ordered where applicable. | Directly consumes Navigation state/validity for return, landing and safe completion decisions. Existing traceability identifies ARCH-DEC-017 as primary and CAP-001 as supporting context. | PARTIAL / DIRECT DEPENDENCY | Keep SYS-REQ-082. Navigation must provide validated state required by the safe-completion chain. |
| SYS-REQ-085 | Under resource contention P0/P1 have priority over P2-P5; P0/P1 include safety, flight control, navigation and C2; lower-priority functions must not obstruct P0/P1. | Navigation is explicitly named as P0/P1. This is a resource/authority constraint, not a Navigation functional requirement. | SUPPORTING / CROSS-CUTTING | Keep SYS-REQ-085. Allocate Navigation as protected critical workload. |
| SYS-REQ-086 | HUB shall provide controlled degradation by priority; P5/P4 first, P3 if necessary; P2 retained as needed for safe mission; P0/P1 retained regardless of low-priority load. | Navigation is explicitly within P0/P1 through SYS-REQ-085. Navigation degradation must therefore remain within controlled degradation/authority boundaries. | SUPPORTING / CROSS-CUTTING | Keep SYS-REQ-086. Navigation-specific degradation behaviour remains in Navigation/Safety design. |
| SYS-REQ-091 | Critical messages shall have controlled allowable latency values; P0/P1 stricter latency than P3-P5; protective action on threshold exceedance. | Navigation produces critical navigation state and validity; latency/freshness can affect whether navigation state is usable. However SYS-REQ-091 is a general critical-latency requirement, not a Navigation-specific latency specification. | SUPPORTING / INDIRECT | Keep SYS-REQ-091. Navigation-specific timing thresholds require lower-level requirement/verification definition. |
| SYS-REQ-093 | After overload removal HUB shall recover constrained services controllably; recovery shall not cause secondary overload; critical functions recover before low-priority functions; degradation/recovery state shall be recorded. | Navigation is protected P0/P1 and therefore falls under controlled resource recovery. This requirement governs recovery/resource behaviour, not Navigation computation itself. | SUPPORTING / INDIRECT | Keep SYS-REQ-093. Preserve linkage to Navigation as critical protected service where applicable. |

## Key findings

1. No SYS-REQ-081/082/085/086/091/093 is semantically equivalent to a Navigation requirement.
2. SYS-REQ-081 and SYS-REQ-082 have the strongest functional dependency on Navigation.
3. SYS-REQ-085 and SYS-REQ-086 establish cross-cutting priority/degradation constraints that explicitly protect Navigation because Navigation is P0/P1.
4. SYS-REQ-091 constrains latency of critical messages; it does not itself define Navigation latency values.
5. SYS-REQ-093 constrains controlled recovery of critical services; it does not itself define Navigation recovery algorithms.
6. Therefore no MERGE operation is justified by this pass.

## Required Navigation closure

The Navigation layer still needs controlled lower-level definition for:
- state model;
- data quality states;
- reference frame;
- Course / Heading / Track semantics;
- navigation deviation;
- freshness/validity thresholds;
- applicable latency thresholds;
- degraded/recovery behaviour;
- verification methods and evidence.

## Decision

```text
SYS-REQ-081 → KEEP + Navigation SUPPORTING allocation
SYS-REQ-082 → KEEP + Navigation DIRECT DEPENDENCY
SYS-REQ-085 → KEEP + Navigation P0/P1 protection
SYS-REQ-086 → KEEP + Navigation degradation boundary
SYS-REQ-091 → KEEP + Navigation timing dependency
SYS-REQ-093 → KEEP + Navigation recovery dependency

NAV-REQ-001..009 → remain CANDIDATE pending source/wording closure
MERGE → NOT AUTHORIZED
```

## Evidence basis

- CAP-001 Navigation definition and safety constraints.
- CRB-NAV-001..005 Navigation baseline.
- SYS-REQ-081, 082, 085, 086, 091, 093 controlled requirement records.
- Existing architecture/capability traceability records.
