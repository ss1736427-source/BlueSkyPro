# CHANGELOG

## 2026-08-31

- Established `PROJECT_RULES.md` as the active project rules document.
- Completed known-damage cleanup and confirmed zero known-damage findings.
- Created the source-preservation and knowledge-base structure.
- Preserved all 98 uploaded source files separately from the navigation layer.

## 2026-10-18

- Сверен указатель состояния проекта с main HEAD `9f60746d6de6d80ad9afc1ee16722843ed02a8ad`; предыдущие HEAD-ссылки помечены как исторические.
- Синхронизированы handover-документ, checkpoint рабочего плана и указатель Master Document Index.
- Исправлен репозиторий по умолчанию для внешнего orchestrator на `ss1736427-source/BlueSkyPro` и README запуска; ограниченный scope CI-контроля явно документирован.
- Runtime-state orchestrator указывает текущий наблюдаемый SHA main без ложного утверждения о прохождении CI для этого SHA.
- Исторические audit records сохранены; параллельный журнал состояния не создавался.
- Изменения предложены в отдельной ветке обслуживания документации; main не менялся и требует обычного PR review и отдельного разрешения на merge.
