# Anomaly Registry

```dataview
TABLE file.link AS Anomaly, id, severity, status, affected_objects
FROM "11_ANOMALIES"
WHERE type = "anomaly"
SORT severity DESC, id ASC
```



