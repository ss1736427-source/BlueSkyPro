# Verification Registry

```dataview
TABLE file.link AS Verification, id, status, verified_object, test_case, result, evidence
FROM "09_VERIFICATION/Results"
WHERE type = "verification"
SORT id ASC
```



