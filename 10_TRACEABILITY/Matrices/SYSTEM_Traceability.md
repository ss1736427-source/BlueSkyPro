# SYSTEM Traceability

## SYSTEM → SYS

```dataview
TABLE id, title, derives_from, criticality, verification_method
FROM "02_SYSTEM/Requirements"
WHERE type = "system_requirement"
SORT id ASC
```

## ARCH → SYS

```dataview
TABLE id, title, type, status
FROM "01_ARCHITECTURE"
WHERE type = "architecture"
SORT id ASC
```

## HUB Core Requirements

```dataview
TABLE id, title, criticality, verification_method
FROM "02_SYSTEM/Requirements"
WHERE type = "system_requirement"
AND contains(derives_from, "[[ARCH-025]]")
SORT id ASC
```

## HUB Module Interface Requirements

```dataview
TABLE id, title, criticality, verification_method
FROM "02_SYSTEM/Requirements"
WHERE type = "system_requirement"
AND contains(derives_from, [[ARCH-026]])
SORT id ASC
```

## Heterogeneous UAV Fleet Requirements

```dataview
TABLE id, title, criticality, verification_method
FROM "02_SYSTEM/Requirements"
WHERE type = "system_requirement"
AND contains(derives_from, [[ARCH-027]])
SORT id ASC
```

## HUB Resource and Safety-Critical Requirements

```dataview
TABLE id, title, criticality, verification_method
FROM "02_SYSTEM/Requirements"
WHERE type = "system_requirement"
AND contains(derives_from, [[ARCH-028]])
SORT id ASC
```

## AI Learning and Continuous Improvement Requirements

```dataview
TABLE id, title, criticality, verification_method
FROM "02_SYSTEM/Requirements"
WHERE type = "system_requirement"
AND contains(derives_from, [[ARCH-029]])
SORT id ASC
```
