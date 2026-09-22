# Controlled Access and Repository Rules

## Repository

**BlueSkyPro** is the working repository.

The review branch is:

`controlled-review-2026-09-22`

This branch is a review surface, not a security boundary.

## Important GitHub limitation

For a repository owned by a personal GitHub account, collaborators receive repository-level access. A branch link does not restrict a collaborator to that branch.

Therefore:

- do not treat the review branch as an access-control mechanism;
- do not place credentials or production secrets in the repository;
- protect `main`;
- grant repository access only to identified GitHub accounts;
- remove access when the review/work period ends.

## Recommended permissions

For a software team that needs to inspect and work with the code, repository-level collaboration can be granted only after the team members are identified.

Do not share:

- account passwords;
- personal access tokens;
- SSH private keys;
- cloud credentials;
- production credentials.

## Baseline rule

`main` is the controlled project baseline.

Proposed changes should be made on working branches and merged only after review.

## Disclosure rule

The project can expose architecture, requirements, interfaces and implementation needed for the agreed engineering task. Access to secrets, production systems and unnecessary proprietary assets is not part of ordinary technical review.

## Offboarding

When a person's work is complete:

1. remove repository access;
2. revoke any project-specific credentials separately;
3. review open Pull Requests and branches;
4. verify that no secrets were added;
5. retain the project history under owner control.

## Ownership

Repository ownership remains with the project owner. Technical collaborators are contributors to the repository, not owners of the project.
