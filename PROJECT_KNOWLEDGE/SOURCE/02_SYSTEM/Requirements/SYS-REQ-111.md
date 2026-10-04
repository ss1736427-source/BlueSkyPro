---
id: SYS-REQ-111
type: system_requirement
title: AI Agent Authority and Proposal Control
status: draft
criticality: critical
verification_method: test
derives_from:
  - "[[ARCH-DEC-046]]"
---

# SYS-REQ-111 — AI Agent Authority and Proposal Control

## Requirement

Система должна обеспечивать явное разделение между AI-анализом, AI-рекомендацией, AI-предложением и авторитетным управлением состоянием системы.

AI-агенты не должны самостоятельно изменять авторитетное состояние миссии, UAV, ресурсов или системы.

## Authority Boundary

AI-агент может:

- анализировать состояние;
- выявлять отклонения;
- формировать рекомендации;
- генерировать варианты;
- формировать предложения;
- оценивать последствия предложенных изменений.

AI-агент не должен самостоятельно:

- изменять авторитетное состояние миссии;
- изменять Safety Engine state;
- отменять Safety Gate;
- обходить Mission Validation;
- выдавать неограниченные команды исполнения;
- самостоятельно утверждать material mission changes.

## Controlled Execution Path

атериальное AI-предложение должно проходить предусмотренный путь:

AI AGENT
→ PROPOSAL
→ VALIDATION
→ SAFETY GATE
→ AUTHORIZATION
→ C++ CORE
→ EXECUTION

и один промежуточный AI-компонент не должен обходить обязательные этапы этого пути.

## Safety Authority

Safety Engine должен сохранять высший приоритет в вопросах безопасности.

AI-агент не должен:

- отменять Safety Engine decision;
- понижать обязательное safety constraint;
- разрешать выполнение заблокированной операции;
- изменять safety-critical state без предусмотренной авторизации.

## Mission Authority

C++ Core должен оставаться владельцем авторитетного runtime mission state.

AI-generated state или proposal не должен становиться авторитетным только вследствие его создания AI-агентом.

## Operator Authority

сли для изменения требуется operator approval, AI-агент не должен обходить это требование.

Система должна сохранять факт:

- необходимости approval;
- предоставления approval;
- отказа;
- лица или роли, предоставившей approval;
- применённой версии предложения.

## Proposal Identity

аждое material AI proposal должно иметь возможность быть однозначно идентифицировано.

олжны сохраняться, где применимо:

- proposal identifier;
- source agent;
- orchestrator task;
- input context;
- affected mission;
- affected resources;
- proposed changes;
- assumptions;
- confidence;
- validation result;
- safety result;
- authorization result;
- execution result.

## Validation

Material AI proposals должны проходить применимые проверки до исполнения.

Минимальный контролируемый путь:

PROPOSAL
→ MISSION VALIDATION
→ READINESS
→ SAFETY GATE
→ AUTHORIZATION
→ EXECUTION

Предложение, не прошедшее обязательную проверку, не должно становиться executable state.
## Rejection

Система должна поддерживать явный отказ AI-предложения.

ричина отказа должна быть трассируема.

тказ может быть вызван:

- failed validation;
- failed safety check;
- missing approval;
- violated constraint;
- resource unavailability;
- invalid proposal;
- expired proposal;
- conflicting proposal;
- system policy.

## Conflicting Proposals

ри наличии нескольких противоречащих предложений система не должна выбирать предложение исключительно на основании AI confidence или optimization score.

онфликт должен передаваться в предусмотренный механизм разрешения, валидации или авторизации.

## Automatic Authorization

втоматическое применение AI-generated proposal допускается только для заранее определённого и авторизованного класса изменений.

Такое автоматическое применение должно оставаться:

- ограниченным;
- проверяемым;
- трассируемым;
- подчинённым Safety Engine;
- подчинённым системным ограничениям.

## No Privilege Escalation

AI-агент не должен получать дополнительные полномочия посредством:

- собственного предложения;
- собственного результата;
- изменения конфигурации;
- изменения feature flag;
- изменения модели;
- изменения knowledge source;
- другого AI-агента.

## Auditability

Система должна сохранять возможность восстановить:

REQUEST
→ AI ANALYSIS
→ PROPOSAL
→ VALIDATION
→ SAFETY DECISION
→ AUTHORIZATION
→ EXECUTION
→ RESULT

ля material decisions история должна быть неизменяемой в пределах применяемой configuration and audit policy.

## Failure Behaviour

При ошибке AI-компонента система должна сохранять authority boundaries.

Ошибки AI не должны:

- автоматически разрешать заблокированное действие;
- изменять safety priority;
- передавать AI полномочия C++ Core;
- обходить operator approval;
- приводить к uncontrolled execution.

## Verification

Проверка должна продемонстрировать:

- AI не может самостоятельно изменить authoritative mission state;
- AI не может обходить Mission Validation;
- AI не может обходить Safety Gate;
- AI не может отменить Safety Engine decision;
- operator approval соблюдается;
- rejected proposal не становится executable;
- conflicting proposals обрабатываются контролируемо;
- automatic authorization ограничено разрешённым классом изменений;
- material AI decisions полностью трассируются.

## Traceability

- [[ARCH-DEC-046]]
- [[SYS-REQ-085]]
- [[SYS-REQ-086]]
- [[SYS-REQ-087]]
- [[SYS-REQ-088]]
- [[SYS-REQ-110]]
- [[ARCH-DEC-007]]
- [[ARCH-DEC-016]]
- [[ARCH-DEC-039]]



