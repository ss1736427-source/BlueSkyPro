# BlueSky PRO — External Orchestrator

Внешний управляющий цикл для разработки BlueSky PRO.

```text
GitHub main → новый SHA → CI именно этого SHA → PASS → 5 секунд → agent adapter → следующий шаг → новый SHA
```

## Запуск на Windows

Требуется Python 3.10+ и `GITHUB_TOKEN` с необходимыми правами GitHub.

Синхронизируй локальный клон:

```cmd
git pull --ff-only
```

Затем задай:

```cmd
set BS_REPO=ss1736427-source/BlueSky-PRO-Knowledge
set BS_BRANCH=main
set BS_POLL_SECONDS=5
set BS_CI_GRACE_SECONDS=5
set BS_AGENT_ADAPTER=E:\Flight Planning\agent_adapter.ps1
set BS_AGENT_EXECUTABLE=<approved-agent-executable>
```

Запуск:

```cmd
python E:\Flight Planning\bluesky_orchestrator.py
```

Либо через PowerShell launcher:

```powershell
.\run_orchestrator.ps1
```

Адаптер `agent_adapter.ps1` передаёт агенту точный проверенный SHA и пути к:
- `00_PROJECT/GITHUB_DEVELOPMENT_PROTOCOL.md`
- `00_PROJECT/BLUE_SKY_PRO_WORKING_RULES.md`

Agent должен вернуть:
- `0` — техническая работа завершена, можно продолжать;
- `42` — требуется решение пользователя, цикл останавливается;
- другой ненулевой код — ошибка агента.

`UNVERIFIED` никогда не считается `PASS`, а старый CI не подтверждает новый SHA.

## Windows PATH

Путь к локальной папке с оркестратором может быть любым. В переменной `BS_AGENT_ADAPTER` указывается фактический путь к `agent_adapter.ps1`.

`BS_AGENT_EXECUTABLE` должен явно указывать на одобренный внешний агент, совместимый с интерфейсом stdin/stdout, используемым адаптером. Автоматический выбор конкретного агента не выполняется.

Этот компонент является внешним управляющим циклом и не принимает архитектурных или продуктовых решений самостоятельно.
