# BS-ARCH — Flight Plan / ЕС ОрВД / REJ Processing

## 1. Scope
REJ is specifically the BlueSky function for automatic handling of rejection of an FPL submitted to the Unified Air Traffic Management System.

It is NOT a generic mission editor.

## 2. Normative basis
Working engineering basis: ТС-13.
Current normative reference for final compliance: ТС-15.
Differences shall be tracked explicitly in the compliance/change log.

The current TS-15 text defines REJ as a message submitted when an FPL departs from its composition rules and cannot be processed by ATS authorities; it specifies fields 3, 7, 8, 9, 10, 13, 15, 16 and 18. ACK is the acceptance-for-automated-processing message. citeturn0search0turn0search1

## 3. Automatic FPL generation
BlueSky shall:
1. derive flight-plan data from the approved mission;
2. construct FPL according to the configured regulatory profile;
3. validate the FPL;
4. submit it through the configured communication channel;
5. record the submitted message and version.

TS-15 defines FPL as the presented flight-plan message and specifies the submission/recipient framework, including dedicated provisions for UAV flight plans. citeturn0search1turn0search4

## 4. ACK
On ACK:
- mark the submitted FPL as accepted for automated processing;
- store the ACK;
- bind it to the exact FPL version;
- update flight-plan status.

## 5. REJ
On REJ:
1. receive and store the REJ;
2. associate it with the exact FPL version;
3. extract the rejection information;
4. identify affected fields/parameters;
5. map the reason to an applicable correction rule;
6. determine correction authority:
   - AUTO-CORRECT;
   - OPERATOR APPROVAL;
   - MANUAL ACTION;
7. generate a corrected FPL candidate;
8. validate the candidate;
9. show the operator the changes;
10. notify the operator;
11. if policy permits automatic resubmission, submit the corrected FPL;
12. otherwise wait for operator approval.

## 6. Change transparency
Every correction shall display:
- field;
- line/segment where applicable;
- old value;
- new value;
- reason;
- source of the correction;
- validation result.

## 7. Version chain
Maintain:
`FPL v1 -> REJ -> correction -> FPL v2 -> ACK/REJ -> ...`

No version may overwrite the previous submitted version.

## 8. Safety and governance
Automatic correction shall not be unlimited. The system shall:
- enforce a maximum retry count;
- prevent infinite REJ loops;
- require operator approval for ambiguous/high-impact changes;
- preserve a complete audit trail;
- allow operator cancellation of automatic resubmission.

## 9. Regulatory separation
Normative rules shall be represented separately from BlueSky automation logic.

`TS-15 Rule -> BlueSky Validator/Correction Rule -> Requirement -> Test`

This prevents project-specific automation from being misrepresented as a regulatory requirement.



