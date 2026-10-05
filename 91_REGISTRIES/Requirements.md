# Requirements Registry

```dataview
TABLE file.link AS Requirement, id, type, status, criticality, verification_method
FROM "01_NORMATIVE/02_Requirements"
WHERE type = "normative_requirement"
SORT id ASC
```

```dataview
TABLE file.link AS Requirement, id, type, status, criticality, verification_method
FROM "02_SYSTEM/Requirements"
WHERE type = "system_requirement"
SORT id ASC
```

```dataview
TABLE file.link AS Requirement, id, type, status, criticality, verification_method
FROM "04_SOFTWARE/Requirements"
WHERE type = "software_requirement"
SORT id ASC
```



