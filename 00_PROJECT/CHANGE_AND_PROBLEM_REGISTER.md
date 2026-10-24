# Change and Problem Register

## Status

CONTROLLED WORKING REGISTER

## Purpose

Record controlled changes and problems affecting project-controlled artifacts, baselines, verification records, configuration-controlled artifacts, or certification evidence.

## Record structure

Each entry shall contain, at minimum:

| Field | Required content |
|---|---|
| Record ID | Unique change or problem identifier |
| Type | CHANGE or PROBLEM |
| Date | Date identified/raised |
| Affected artifact | Controlled file, record, baseline, or process |
| Source / trigger | Origin of the change or observed condition |
| Impact | Requirements, safety, configuration, verification, certification, or other applicable impact |
| Disposition | Proposed/current disposition |
| Approval status | Current approval state |
| Implementation | Commit or other implementation reference |
| Verification | Required verification or re-verification and status |
| Closure | Open/closed status and closure reference |

## Register

| Record ID | Type | Date | Affected artifact | Source / trigger | Impact | Disposition | Approval status | Implementation | Verification | Closure |
|---|---|---|---|---|---|---|---|---|---|---|
| CHG-2026-10-24-NOTAM-TIME | CHANGE | 2026-10-24 | `02_SYSTEM_DESIGN/PLANNING/NOTAM_PROHIBITED_ZONE_VALIDATION_001.md`; `04_SOFTWARE/PLANNING/notam_prohibited_zone_validator_test.cpp` | Requirement confirmed that NOTAM restrictions must apply according to the actual mission/flight time, not merely wall-clock time or an assumed mission-start time. | Planning safety, map presentation, route optimization and validation consistency. | Record the temporal-applicability requirement and add validity-interval boundary tests. Full integration of mission-time display and timed route occupancy remains outstanding. | Requirement clarified; implementation/verification incomplete. | [`8f42a16`](https://github.com/ss1736427-source/BlueSkyPro/commit/8f42a1675161bd1218aa3fdf1d3e3c01491b60ff) | Boundary tests added; GitHub workflow checks on implementation commit passed, but this does not verify mission-time map filtering or timed segment occupancy. | OPEN |


## Control rule

Entries shall remain traceable to the authoritative master register and applicable baseline. Repository history is retained as the implementation record for committed repository changes.
