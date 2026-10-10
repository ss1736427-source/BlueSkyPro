# Traceability Health

## Requirements without verification

```dataview
TABLE file.link AS Requirement, id, status, criticality
FROM ""
WHERE (type = "normative_requirement" OR type = "system_requirement" OR type = "software_requirement" OR type = "safety_requirement")
AND (verified_by = null OR length(verified_by) = 0)
SORT id ASC
```

## Software requirements without implementation

```dataview
TABLE file.link AS Requirement, id, status, implements
FROM "04_SOFTWARE/Requirements"
WHERE type = "software_requirement"
AND (implements = null OR length(implements) = 0)
SORT id ASC
```



