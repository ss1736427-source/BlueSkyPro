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

No change or problem records are entered by this document unless they have been separately identified and controlled.

## Control rule

Entries shall remain traceable to the authoritative master register and applicable baseline. Repository history is retained as the implementation record for committed repository changes.
