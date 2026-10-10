# Реестр программных требований

```dataview
TABLE
    id AS "ID",
    status AS "Статус",
    criticality AS "Критичность",
    parent AS "Родительское требование",
    verification_method AS "Метод верификации",
    baseline AS "Базовая версия"
FROM "04_SOFTWARE/SW_Requirements"
WHERE type = "software_requirement"
SORT id ASC
```
