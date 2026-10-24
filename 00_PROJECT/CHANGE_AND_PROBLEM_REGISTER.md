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
| CHG-2026-10-24-NOTAM-TIME | CHANGE | 2026-10-24 | `02_SYSTEM_DESIGN/PLANNING/NOTAM_PROHIBITED_ZONE_VALIDATION_001.md`; `04_SOFTWARE/PLANNING/notam_prohibited_zone_validator.hpp/.cpp`; route timing/wind integration and optimizer candidate-generation components; associated tests and HMI presentation | User clarified that NOTAM applicability must be evaluated against wind-adjusted route occupancy times, and conflicting routes must be rebuilt using the optimizer's best feasible detour. | Planning safety, time-dependent prohibited-zone avoidance, wind-aware trajectory timing, optimizer objective preservation, map presentation and validation consistency. | Specify the required sequence: candidate generation → wind application and time-profile calculation → combined geometry/altitude/time validation → deterministic conflict feedback → rebuild with best feasible detour under the existing objective → recalculate wind/timing and revalidate all zones. Keep UTC-normalized inclusive validity boundaries and fail closed for unreliable required timing/validity inputs. | Requirement clarified and design updated; end-to-end implementation and verification remain incomplete. | Design requirement update: [`2bb7679`](https://github.com/ss1736427-source/BlueSkyPro/commit/2bb7679eb858e143a82ca1d148f9a15f6ccc805b). Prior boundary-test commit: [`8f42a16`](https://github.com/ss1736427-source/BlueSkyPro/commit/8f42a1675161bd1218aa3fdf1d3e3c01491b60ff). | Existing boundary tests do not verify wind-adjusted segment occupancy, optimizer conflict feedback, shortest feasible detours, or full revalidation after route changes. These remain required verification items. | OPEN |


## Control rule

Entries shall remain traceable to the authoritative master register and applicable baseline. Repository history is retained as the implementation record for committed repository changes.
