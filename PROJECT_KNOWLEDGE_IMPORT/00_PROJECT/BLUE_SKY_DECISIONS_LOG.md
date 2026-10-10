# BLUE SKY — DECISIONS LOG

> Журнал утверждённых архитектурных, safety, regulatory и certification решений.

## DEC-001 — Aviation System Classification
**Status:** APPROVED

BlueSky рассматривается как авиационная система, а не только как программное обеспечение.

**Impact:** Certification Basis, SRS, Architecture, Safety, Verification.

---

## DEC-002 — CONFIGURATION-A
**Status:** APPROVED

Первый профиль продукта:
- `CONFIGURATION-A`
- MTOM ≤30 kg
- A-MR
- A-FW
- A-VT

Порог ≤30 kg не является сам по себе заявлением о сертификации.

---

## DEC-003 — Separate >30 kg Certification Path
**Status:** APPROVED

БАС >30 kg не включаются автоматически в текущий certification path. Архитектура сохраняется расширяемой.

---

## DEC-004 — Mission-Specific UAV Readiness
**Status:** APPROVED

Готовность БАС определяется применительно к конкретной миссии: оборудование, техническое состояние, ресурс, пригодность и иные условия.

---

## DEC-005 — Mandatory Reconnaissance on Critical Data Gap
**Status:** APPROVED

При отсутствии необходимых внешних/объектных данных система должна инициировать разведку до основной миссии.

Flow:
`READY → ВЗЛЁТ → РАЗВЕДКА → ПОЛУЧЕНИЕ ДАННЫХ → ПРОВЕРКА ДАННЫХ → ПЕРЕРАСЧЁТ МИССИИ → ОСНОВНАЯ МИССИЯ`

---

## DEC-006 — STBY
**Status:** APPROVED

Резервные/ожидающие БАС имеют отдельное видимое состояние `STBY`.

---

## DEC-007 — C2 Degradation Logic
**Status:** APPROVED

Ухудшение C2 не означает автоматический возврат. Система сначала адаптирует коммуникационный канал и оценивает возможность продолжения.

---

## DEC-008 — Dynamic Return
**Status:** APPROVED

Return trajectory continuously recalculated using actual position, restrictions, obstacles, wind, resource, technical state and other relevant factors.

---

## DEC-009 — Emergency Priority
**Status:** APPROVED

При невозможности безопасного продолжения приоритет:
1. сохранение БАС;
2. сохранение уже собранных данных;
3. дальнейший сбор данных — только если безопасен.

---

## DEC-010 — Mission / UAV Time Model
**Status:** APPROVED

Общий Mission TOT не заменяет индивидуальные TOT и WP times каждого БАС.

---

## DEC-011 — Immutable Archive
**Status:** APPROVED

Первичная запись миссии/полёта неизменяема. Исправления и производные данные создаются отдельными записями.

---

## DEC-012 — C2 Provider Boundary
**Status:** APPROVED

BlueSky должен поддерживать DIRECT-C2 и PROVIDER-C2 и иметь контролируемый интерфейс с внешним C2 provider.

BlueSky не считается C2 provider автоматически.

---

## DEC-013 — Regulatory Gap Handling
**Status:** APPROVED

Неустановленную/неподтверждённую отдельную регуляторную процедуру обозначать `REG-GAP / REGULATORY WATCH`, а не превращать предположение в нормативный факт.

---

## DEC-014 — Certification Traceability
**Status:** APPROVED

Требования должны быть связаны по цепочке:

`Regulation → Applicability → Requirement → Design/Interface → Verification → Evidence`

---

## DEC-015 — Current C2 Work Package
**Status:** IN WORK

Следующий основной пакет:
декомпозиция Приказа №142 в требования `C2-REQ-*` и их трассировка через SRS, ICD, Safety, Verification Plan, RTM и Evidence.



