# Test Registry

```dataview
TABLE file.link AS Test, id, status, verifies, expected_result, result
FROM "09_VERIFICATION/Test_Cases"
WHERE type = "test_case"
SORT id ASC
```



