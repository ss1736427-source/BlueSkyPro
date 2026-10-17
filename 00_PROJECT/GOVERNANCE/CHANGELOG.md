# CHANGELOG

## 2026-08-31

- Established `PROJECT_RULES.md` as the active project rules document.
- Completed known-damage cleanup and confirmed zero known-damage findings.
- Created the source-preservation and knowledge-base structure.
- Preserved all 98 uploaded source files separately from the navigation layer.

## 2026-10-18

- Сверен указатель состояния проекта с main HEAD `c2b3971a1b188657a60166e900dcdbb9dd0f77ff`; предыдущие HEAD-ссылки помечены как исторические.
- Синхронизированы handover-документ, checkpoint рабочего плана и указатель Master Document Index.
- Исправлены репозиторий по умолчанию для внешнего orchestrator на `ss1736427-source/BlueSkyPro` и README запуска; ограниченный scope CI-контроля явно документирован.
- Runtime-state orchestrator указывает текущий наблюдаемый SHA main без ложного утверждения о прохождении CI для этого SHA.
- Исторические audit records сохранены; параллельный журнал состояния не создавался.
- Изменения предложены в отдельной ветке обслуживания документации; main не менялся и требует обычного PR review и отдельного разрешения на merge.
