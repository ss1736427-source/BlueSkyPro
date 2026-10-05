---
id: ADMINISTRATOR_MENU_STRUCTURE_001
type: architecture
status: BASELINE
version: 0.1
---

# ADMINISTRATOR — минимальная структура меню

## 1. Принцип

Первый уровень показывает только основные пользовательские функции. Технические детали раскрываются только после выбора соответствующего направления.

```text
ADMINISTRATOR
│
├── SYSTEM
├── USERS & ROLES
├── UAV & CONFIGURATION
├── DATA & PROCESSING
├── EXTERNAL CONNECTIONS
└── LOG / AUDIT
```

## 2. SYSTEM

Назначение: управление самой системой.

```text
SYSTEM
├── System status
├── General settings
├── Workspaces
├── Localization
└── Update / version
```

Пользователь видит только параметры, относящиеся к его полномочиям.

## 3. USERS & ROLES

Назначение: управление пользователями и их функциональными возможностями.

```text
USERS & ROLES
├── Users
│   ├── User profile
│   └── Access status
│
├── Roles
│   ├── Administrator
│   ├── Technical
│   ├── Engineer
│   └── Pilot
│
├── Combined functions
│   └── Role combination
│
└── Permissions
    ├── Function access
    ├── Data access
    └── External systems access
```

Один пользователь может иметь несколько функций одновременно. Совмещение функций не требует создания нескольких учётных записей.

## 4. UAV & CONFIGURATION

Назначение: управление моделями БПЛА, оборудованием и конфигурациями.

```text
UAV & CONFIGURATION
├── UAV Models
│   ├── Reference models
│   └── Custom
│
├── Equipment
│   ├── Batteries
│   ├── Payloads
│   ├── Cameras
│   ├── Navigation
│   └── Communications
│
├── Configurations
│   ├── Automatic
│   └── Custom manual
│
└── Compatibility
    ├── Compatible
    ├── Restricted
    └── Incompatible
```

При автоматическом подборе пользователю предлагаются только совместимые компоненты. Несовместимые компоненты не отображаются как варианты выбора.

Custom допускает ручную нестандартную конфигурацию, включая дополнительные батареи и нестандартное оборудование, при наличии соответствующих прав.

## 5. DATA & PROCESSING

Назначение: работа с собранными данными и подготовка их к дальнейшему использованию.

```text
DATA & PROCESSING
├── Datasets
├── Collection
├── Validation
├── Preparation
├── Export
├── External Processing
└── Results
```

Цепочка данных:

```text
MISSION → DATASET → VALIDATION → PREPARATION → EXPORT
                                             │
                                             ▼
                                    EXTERNAL PROCESSING
                                             │
                                             ▼
                                          RESULT
```

## 6. EXTERNAL CONNECTIONS

Назначение: управление внешними источниками и системами.

```text
EXTERNAL CONNECTIONS
├── Navigation Data
├── NOTAM
├── Weather
├── Communications
├── External Software
├── Data Services
└── Connection Profiles
```

Для каждого подключения должны существовать статус, профиль, права доступа и журнал обмена.

## 7. LOG / AUDIT

Назначение: контроль событий и действий, имеющих значение для эксплуатации и администрирования.

```text
LOG / AUDIT
├── System events
├── User actions
├── Configuration changes
├── Data exchange
├── External processing
└── Audit records
```

## 8. User-specific visibility

Полная структура возможностей не должна автоматически отображаться каждому пользователю.

```text
SYSTEM CAPABILITY
      ↓
ROLE / PERMISSION
      ↓
USER PROFILE
      ↓
VISIBLE MENU
      ↓
PERSONAL WORKSPACE
```

Настройка меню изменяет только представление доступных функций. Она не изменяет права доступа.

## 9. Глубина меню

Основное правило: не более необходимой глубины.

```text
LEVEL 1 — что нужно сделать
LEVEL 2 — область функции
LEVEL 3 — конкретная операция / объект
LEVEL 4+ — только специализированные настройки
```

Обычный пользователь не должен проходить через административные и технические уровни для выполнения своей рабочей операции.

## 10. Связи

```text
USERS & ROLES
      │
      ├──────────────┐
      ▼              ▼
UAV & CONFIG     DATA & PROCESSING
      │              │
      └──────┬───────┘
             ▼
    EXTERNAL CONNECTIONS
             │
             ▼
           HUB / DUM
             │
             ▼
       DATA EXCHANGE
             │
             ▼
     EXTERNAL PROCESSING
```

## 11. UI rule

ADMINISTRATOR должен строиться не вокруг внутренних модулей системы, а вокруг действий пользователя.

Пример:

`DATA & PROCESSING → Export → выбрать назначение → система показывает только совместимые форматы/адаптеры → Export.`

Не требуется показывать пользователю внутренние протоколы, API, адаптеры или служебные параметры, если они не нужны для его функции.
