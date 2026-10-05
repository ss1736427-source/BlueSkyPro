# Traceability Matrix

## SYS → SW

```dataview
TABLE id, title, derives_from
FROM "04_SOFTWARE/Requirements"
WHERE type = "software_requirement"
SORT id ASC
```

## SW → ARCH

```dataview
TABLE id, title, satisfies
FROM "04_SOFTWARE/Architecture"
WHERE type = "architecture"
SORT id ASC
```

## ARCH → DES

```dataview
TABLE id, title, refines
FROM "04_SOFTWARE/Design"
WHERE type = "design"
SORT id ASC
```

## DES → MOD

```dataview
TABLE id, title, implements
FROM "04_SOFTWARE/Modules"
WHERE type = "module"
SORT id ASC
```

## SW → TEST

```dataview
TABLE id, title, verifies
FROM "09_VERIFICATION/Test_Cases"
WHERE type = "test_case"
SORT id ASC
```

## TEST → VER

```dataview
TABLE id, title, verified_object, test_case, result
FROM "09_VERIFICATION/Results"
WHERE type = "verification"
SORT id ASC
```

## VER → EVID

```dataview
TABLE id, title, supports, verification, status
FROM "15_CERTIFICATION/Evidence"
WHERE type = "evidence"
SORT id ASC
```


