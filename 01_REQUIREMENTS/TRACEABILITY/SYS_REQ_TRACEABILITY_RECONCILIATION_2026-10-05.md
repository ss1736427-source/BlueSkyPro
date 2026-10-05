# SYS-REQ Traceability Reconciliation

Current main contains 100 SYS-REQ files. Absent identifiers: 024, 031, 034, 039, 044, 045, 046, 047, 048, 056, 057, 058. A repository commit search for each identifier returned no matching commit. They are not recreated by this audit.

## Architecture

SYS-REQ-ARCHITECTURE-COVERAGE contains 95 rows. Existing requirements absent from that central matrix: SYS-REQ-054, SYS-REQ-055, SYS-REQ-110, SYS-REQ-111, SYS-REQ-112.

SYS-REQ-110/111/112 already have dedicated capability-traceability records; their absence from the central matrix is an index defect. SYS-REQ-054/055 are insurance requirements and currently have no central architecture allocation; treat them as architecture gaps until explicitly reconciled.

## Verification

SYSREQ_Verification_Coverage declares scope SYS-REQ-059..109 and is therefore not a complete system verification matrix. TEST-001 and other verification records exist outside that scope.

Requirement-link metadata is inconsistent: TEST-001 uses the verifies field, while TEST-076 uses the requirements field. The controlled schema should use one field and legacy variants must be reconciled explicitly.

## Controlled chain

Requirement -> Verification Method -> Verification Case -> Execution Result -> Evidence -> Configuration.

Architecture allocation alone does not establish verification.

## Next actions

1. Regenerate central architecture coverage from the 100 existing SYS-REQ files.
2. Add explicit GAP rows for SYS-REQ-054 and SYS-REQ-055.
3. Integrate existing 110/111/112 capability traces into the central architecture index.
4. Regenerate SYS-REQ verification coverage from TEST-* front matter.
5. Normalize requirement linkage to one controlled field.
6. Link executed results, evidence and configuration.
7. Reconcile the 12 absent identifiers against authoritative source history before any decision on disposition.

No requirement wording or identity is changed by this audit.
