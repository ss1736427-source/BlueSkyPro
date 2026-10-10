# Evidence Registry

```dataview
TABLE file.link AS Evidence, id, evidence_type, status, supports, verification, review_status
FROM "15_CERTIFICATION/Evidence"
WHERE type = "evidence"
SORT id ASC
```



