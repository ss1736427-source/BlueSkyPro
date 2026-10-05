# System Requirements Registry

```dataview
TABLE file.link AS Requirement, id, status, criticality, verification_method
FROM "02_SYSTEM/Requirements"
WHERE type = "system_requirement"
SORT id ASC
```


