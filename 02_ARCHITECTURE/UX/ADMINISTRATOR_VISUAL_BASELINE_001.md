# BlueSky PRO HMI — Administrator Visual Baseline

**Status:** approved visual direction for iterative design, 2026-10-20  
**Scope:** Administrator workspace, starting with External Integrations  
**Authority:** user-approved concept image in the conversation. This document records the design direction; it does not claim the concept image itself is a production QML screen.

## Approved visual principle

Use the first, larger-card concept as the baseline. It is preferred over the denser list/dashboard variation.

- Dark navy/charcoal aviation interface with restrained BlueSky cyan accents.
- Fixed left navigation column; selected section is clearly highlighted.
- Top header remains visually quiet and contains only navigation context and essential account/session controls.
- Main work area is wide and uncluttered.
- Use large, readable cards with generous spacing, a clear icon, short title, one-line/two-line description, and a concise status.
- Keep the number of simultaneously visible elements low. Do not put all integration subtypes, credentials, diagnostics, counts, charts and actions on one screen.
- No decorative metrics or invented health counts. Statuses must come from actual service checks.
- One obvious primary action per screen, such as “Добавить источник”.
- Use progressive disclosure: selecting a category opens its details; adding a provider opens a step-by-step wizard.
- Preserve the complete functional tree in expandable navigation, but do not expose every leaf at once.
- Keep Russian labels short and consistent; avoid dense abbreviations unless they are established technical terms.

## Administrator navigation principle

The sidebar groups capabilities by responsibility:
1. Обзор системы
2. Пользователи и роли
3. Системное администрирование
4. Внешние интеграции
5. Данные и обработка
6. Документы и соответствие
7. Журнал аудита
8. Обслуживание и диагностика

The full technical UAV catalogue/configuration branch belongs to the engineering/technical domain, not the Administrator's top-level working area. Administrator navigation may expose the required access/configuration governance but must not imply that the Administrator owns technical decisions assigned to ENGINEER.

Each expandable group reveals concise child items. The right-hand workspace displays one selected task at a time and keeps the sidebar fixed.

## External Integrations landing page

Use large cards for high-level source domains, not a separate card for every endpoint or protocol:
- NOTAM / AIP
- Погода (WX)
- Карты и GIS
- Рельеф и препятствия
- БПЛА и автопилоты
- Связь и C2
- Страхование
- Прочие интеграции

Cards show only a short description, a grounded status, and a navigation affordance. More detailed domains (airspace, landing sites, GNSS/RTK/PPK, traffic/UTM, ATM, payload/video, flight records, external APIs, synchronization and exchange journal) appear after opening the relevant group/category, or within the provider wizard, not all on the landing screen.

## Integration wizard

Progressively ask only for the next required item:
1. Choose integration/source type.
2. Ask for channel/endpoint/address in the correct format.
3. Validate the entered endpoint and explain errors in plain language.
4. Ask for credentials only if that provider requires them.
5. Test authentication.
6. Read a sample and validate schema/format, timestamps and data usability.
7. Report either a real success state or a specific failure and recommended correction.

A successful network connection alone is not “integration ready”; the system must confirm it can read and interpret valid data. Distinguish live sources from Virtual Flight fixtures and synthetic test data.

## Implementation status and constraints

- This is a design baseline for gradual refinement, not a statement that the UI is implemented.
- Preserve the full repository-defined functional scope while simplifying what is visible at any one time.
- Implement the QML workspace and responsive card layout only after the current repository files are reconciled; do not silently drop existing functions.
- Validate the resulting screen in the Qt HMI build and CI.
