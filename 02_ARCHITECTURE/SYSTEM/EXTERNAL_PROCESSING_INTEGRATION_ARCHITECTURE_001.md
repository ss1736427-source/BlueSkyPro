---
id: ARCH-DEC-EXTERNAL-PROCESSING-001
type: architecture
title: External Processing Integration Architecture
status: DRAFT
version: 0.1
---

# ARCH-DEC-EXTERNAL-PROCESSING-001 — External Processing Integration Architecture

## 1. Назначение

BlueSky PRO shall provide a controlled integration path for transfer of collected and prepared mission data to external processing software and return of processing results.

BlueSky PRO is not required to implement every specialized processing algorithm internally. External processing systems are treated as separate systems and integrated through controlled interfaces.

## 2. Архитектурный принцип

Общий путь:

`MISSION → DATASET → DATA MANAGER → EXPORT PACKAGE → ADAPTER / INTERFACE → EXTERNAL PROCESSING → RESULT PACKAGE → DATA MANAGER → BLUE SKY PRO`

Для критических и значимых операций должна сохраняться трассируемость:

`Source → HUB → Destination → Processing → Result`

## 3. Внешние категории ПО

Первоначальный каталог интеграций должен предусматривать классы:

- Photogrammetry
- GIS
- Point Cloud Processing
- Orthomosaic / DSM / DTM Processing
- 3D Reconstruction
- Video / Image Processing
- специализированная аналитика
- пользовательское / Custom Processing

Конкретные продукты подключаются через адаптеры или совместимые стандартные форматы.

## 4. Базовые кандидаты для интеграции

Кандидаты для отдельной квалификации интерфейсов:

- Agisoft Metashape — фотограмметрия и 3D-реконструкция
- Pix4D — фотограмметрия и картографическая обработка
- DJI Terra — обработка данных совместимых платформ
- CloudCompare — обработка и анализ облаков точек
- QGIS — GIS-анализ и картография
- ArcGIS Pro — профессиональный GIS-анализ

Наличие продукта в каталоге не означает его автоматическую эксплуатационную квалификацию. Для каждого интерфейса должен существовать статус совместимости и верификации.

## 5. Data Manager

DUM / Data Manager является управляющей точкой получения, синхронизации, версионирования и распространения данных.

Для внешней обработки он обеспечивает:

- идентификацию набора данных;
- контроль источника;
- контроль времени получения;
- проверку актуальности;
- нормализацию;
- валидацию;
- подготовку экспортного набора;
- регистрацию операции;
- импорт результатов;
- связывание результата с исходной миссией.

## 6. Export Package

Экспортный пакет должен иметь идентификатор и manifest.

Пример:

`BS_DATASET_<ID>/`

- `images/`
- `navigation/`
- `camera/`
- `mission/`
- `geospatial/`
- `metadata/`
- `manifest.json`

Manifest должен, где применимо, содержать:

- Dataset ID
- Mission ID
- UAV
- UAV configuration
- Payload
- Camera
- Date/time
- Coordinate system
- Altitude reference
- RTK/PPK status
- GNSS quality
- image count
- navigation data identity
- software/configuration version
- dataset version
- checksum

## 7. Два режима работы

### 7.1 Automatic

Пользователь выбирает задачу обработки.

Система:

1. определяет требуемый тип данных;
2. определяет доступные совместимые внешние обработчики;
3. проверяет профиль интеграции;
4. формирует пакет;
5. передаёт данные;
6. регистрирует операцию;
7. принимает результат.

### 7.2 Manual / Custom

Для инженерного и расширенного пользователя допускается ручное формирование экспортного пакета:

- выбор данных;
- формат;
- CRS;
- metadata;
- navigation;
- RTK/PPK;
- camera data;
- destination;
- профиль внешнего ПО.

## 8. Import Result

Результаты внешней обработки должны возвращаться как отдельный Result Package.

Примеры:

- orthomosaic
- DSM
- DTM
- point cloud
- 3D model
- GIS layers
- processing report
- quality report

Результат должен сохранять связь с:

`Mission → Dataset → Export → External Processing → Result`

## 9. Adapter Architecture

Внешние системы не должны напрямую зависеть от внутренних компонентов BlueSky PRO.

Рекомендуемая схема:

`Data Manager → Integration API → Adapter → External Software`

Каждый адаптер определяет:

- supported input;
- supported output;
- mapping;
- interface/protocol;
- software version;
- configuration;
- compatibility;
- error handling;
- test status.

Добавление нового внешнего ПО не должно требовать изменения BlueSky Core при сохранении общего интерфейсного контракта.

## 10. Форматы обмена

Архитектура должна поддерживать стандартные форматы, применимые к соответствующей задаче.

Примеры:

- JPEG / TIFF
- GeoTIFF
- CSV
- JSON
- GeoJSON
- LAS / LAZ
- PLY
- SHP
- KML / KMZ
- PDF reports
- другие форматы через зарегистрированный adapter profile

Конкретный формат выбирается профилем задачи и внешнего обработчика.

## 11. Безопасность и граница доверия

Внешнее ПО является отдельным trust domain.

Необходимо:

- аутентифицировать защищённые интерфейсы;
- авторизовать операции;
- передавать только необходимые данные;
- не хранить credentials в mission data или QML;
- разделять operational / test / simulation / sandbox endpoints.

## 12. Ошибки и отказоустойчивость

Для внешних интеграций должны обрабатываться:

- timeout;
- unavailable;
- malformed data;
- authentication failure;
- authorization failure;
- protocol error;
- incompatible version;
- stale data;
- rate limit;
- dependency failure.

Общий путь:

`DETECT → CLASSIFY → WARN / DEGRADE / CRITICAL → RESPOND → RECOVER / REPLAN / ABORT`

Повторные операции должны иметь контролируемую retry policy и idempotency, где применимо.

## 13. Audit / Traceability

Существенные внешние операции должны регистрироваться.

Минимальный состав:

- source;
- destination;
- operation;
- user identity;
- timestamp;
- result;
- error;
- correlation ID.

Трассировка:

`MISSION → CORE OPERATION → ADAPTER → EXTERNAL REQUEST → EXTERNAL RESPONSE → CORE RESULT`

## 14. Observability

Для каждого подключения должны быть доступны, где применимо:

- connection;
- availability;
- latency;
- throughput;
- errors;
- retries;
- queue depth;
- data freshness;
- adapter version;
- external software version.

## 15. Administrator

В ADMINISTRATOR должен быть раздел:

`DATA → EXTERNAL PROCESSING`

В нём:

- External Software Catalog
- Connections
- Adapter Profiles
- Data Exchange Profiles
- Export
- Import
- Access Rights
- Verification Status
- Audit / Exchange Journal

Администратор определяет, какие интеграции доступны каким пользователям.

## 16. User-specific visibility

Полный функционал системы сохраняется независимо от пользовательского представления.

`SYSTEM CAPABILITY → ROLE PERMISSION → USER PROFILE → PERSONAL WORKSPACE`

Пользователь видит только разрешённые ему функции и может настроить отображение доступных функций под свою рабочую задачу.

## 17. Existing architecture links

Документ должен быть связан с:

- DUM / Data Update Manager
- HUB Core
- Data Governance / Data Lineage
- Mission Data Lifecycle
- ADMINISTRATOR
- Configuration Manager
- External Data Integration
- Simulation Adapter
- Evidence / Audit
- Traceability

## 18. Решение

Зафиксировать внешний processing как отдельный интеграционный контур BlueSky PRO с единым Data Exchange Contract, Adapter Architecture, Export Package и Result Package.

Интеграция с конкретным внешним ПО осуществляется через зарегистрированный профиль совместимости и проходит необходимую проверку до использования в эксплуатационном контуре.
