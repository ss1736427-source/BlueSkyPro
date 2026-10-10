---
id: VER-INT-001
type: verification_integrity
title: SYS-REQ Verification Coverage Matrix
status: draft
version: 0.1
---

# VER-INT-001 — SYS-REQ Verification Coverage Matrix

## 1. азначение

атрица предназначена для проверки покрытия системных требований verification/test cases.

Текущий scope: SYS-REQ-059…109.

## 2. Verification Status

опустимые статусы:

- GAP — требование не имеет verification coverage
- PARTIAL — покрытие существует, но недостаточно
- COVERED — имеется соответствующий verification method/test case
- VERIFIED — имеется фактический verification result/evidence

## 3. ARCH-025 — HUB Core

| SYS-REQ | Verification Method | Coverage |
|---|---|---|
| SYS-REQ-059 | test | COVERED |
| SYS-REQ-060 | test | COVERED |
| SYS-REQ-061 | test | COVERED |
| SYS-REQ-062 | test | COVERED |
| SYS-REQ-063 | test | COVERED |
| SYS-REQ-064 | test | COVERED |
| SYS-REQ-065 | test | COVERED |
| SYS-REQ-066 | test | COVERED |

## 4. ARCH-026 — Module Interfaces

| SYS-REQ | Verification Method | Coverage |
|---|---|---|
| SYS-REQ-067 | test | COVERED |
| SYS-REQ-068 | test | COVERED |
| SYS-REQ-069 | test | COVERED |
| SYS-REQ-070 | test | COVERED |
| SYS-REQ-071 | test | COVERED |
| SYS-REQ-072 | test | COVERED |
| SYS-REQ-073 | test | COVERED |
| SYS-REQ-074 | test | COVERED |

## 5. ARCH-027 — Heterogeneous UAV Fleet

| SYS-REQ | Verification Method | Coverage |
|---|---|---|
| SYS-REQ-075 | test | COVERED |
| SYS-REQ-076 | test | COVERED |
| SYS-REQ-077 | test | COVERED |
| SYS-REQ-078 | test | COVERED |
| SYS-REQ-079 | test | COVERED |
| SYS-REQ-080 | test | COVERED |
| SYS-REQ-081 | test | COVERED |
| SYS-REQ-082 | test | COVERED |
| SYS-REQ-083 | test | COVERED |

## 6. ARCH-028 — Resource and Safety-Critical Load Management

| SYS-REQ | Verification Method | Coverage |
|---|---|---|
| SYS-REQ-084 | test | COVERED |
| SYS-REQ-085 | test | COVERED |
| SYS-REQ-086 | test | COVERED |
| SYS-REQ-087 | test | COVERED |
| SYS-REQ-088 | test | COVERED |
| SYS-REQ-089 | test | COVERED |
| SYS-REQ-090 | test | COVERED |
| SYS-REQ-091 | test | COVERED |
| SYS-REQ-092 | test | COVERED |
| SYS-REQ-093 | test | COVERED |

## 7. ARCH-029 — AI Learning and Continuous Improvement

| SYS-REQ | Verification Method | Coverage |
|---|---|---|
| SYS-REQ-094 | inspection | COVERED |
| SYS-REQ-095 | test | COVERED |
| SYS-REQ-096 | test | COVERED |
| SYS-REQ-097 | inspection | COVERED |
| SYS-REQ-098 | test | COVERED |
| SYS-REQ-099 | test | COVERED |
| SYS-REQ-100 | test | COVERED |
| SYS-REQ-101 | analysis | COVERED |
| SYS-REQ-102 | analysis | COVERED |
| SYS-REQ-103 | test | COVERED |
| SYS-REQ-104 | inspection | COVERED |
| SYS-REQ-105 | analysis | COVERED |
| SYS-REQ-106 | test | COVERED |
| SYS-REQ-107 | test | COVERED |
| SYS-REQ-108 | test | COVERED |
| SYS-REQ-109 | inspection | COVERED |

## 8. Current Assessment

се 51 SYS-REQ включены в verification scope.

а данном этапе GAP означает отсутствие подтверждённого test case / verification evidence в текущей матрице.

GAP не означает, что требование невозможно проверить.

## 9. Next Step

ля каждого GAP должен быть создан соответствующий verification/test case с уникальным ID.

осле выполнения test case результат должен быть связан с требованием и verification evidence.



