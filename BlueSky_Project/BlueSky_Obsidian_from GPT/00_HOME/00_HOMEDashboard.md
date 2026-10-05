# Flight Planner — панель сертификации

## Требования

```dataview
TABLE
    type AS "Тип",
    status AS "Статус",
    criticality AS "Критичность"
FROM ""
WHERE contains(type, "requirement")
SORT id ASC
```

## Требования без метода верификации

```dataview
TABLE
    id,
    status,
    verification_method
FROM ""
WHERE contains(type, "requirement")
AND verification_method = null
```

## Открытые аномалии

```dataview
TABLE
    id,
    severity,
    status
FROM "11_ANOMALIES"
WHERE type = "anomaly"
AND status != "closed"
SORT severity DESC
```

## Испытания

```dataview
TABLE
    id,
    status
FROM "09_VERIFICATION/Test_Cases"
WHERE type = "test_case"
SORT id ASC
```
