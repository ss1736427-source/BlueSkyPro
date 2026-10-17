# BlueSky PRO — AI Working State and Checkpoint Register

## Purpose

This file is the single durable handoff record for work performed by ChatGPT or another automated assistant. It prevents loss of confirmed checkpoints, decisions, useful findings, and the exact branch/commit needed to continue.

**This file is a register, not a substitute for Git history, requirements, verification evidence, or PR review.** GitHub is the source of truth for repository state. The register must link to the source records and must never claim a test passed without evidence.

## Mandatory entry rule

Update this file whenever a meaningful checkpoint is reached, before switching tasks/conversations, and whenever branch/PR/verification status materially changes. The update itself must be committed in the same task branch or documentation PR. Do not leave the only copy of important findings in chat.

Each entry must distinguish:
- **CONFIRMED** — directly checked in the repository, a tool result, CI, or a user test.
- **INFERRED** — a reasoned interpretation not yet verified.
- **UNVERIFIED** — not checked or the evidence is unavailable.
- **BLOCKED** — progress requires a missing input or user decision.

Never delete a previous checkpoint to make the current state look cleaner. Append a new entry and mark old entries superseded with a link to the replacement.

## Current canonical pointer

> Fill and verify these fields at the start of every work session. Do not infer them from chat history.

- Repository: `ss1736427-source/BlueSkyPro`
- Canonical integration branch: `main`
- Current `main` SHA: **UNVERIFIED until fetched from GitHub at session start**
- Active task branch: **must be selected and verified for each task**
- Active PR: **must be looked up and verified for each task**
- Last fully confirmed checkpoint: **see Checkpoint Log below**
- Unfinished tests / blockers: **must be refreshed from the active PR and CI**

## Active work register

Maintain one row for every task that is open, in progress, awaiting CI, or awaiting user validation. Do not list a task as active merely because it exists in chat history; verify its GitHub status.

| Task / PR | Head branch | Base branch | Exact head SHA | State | Next action | Evidence |
|---|---|---|---|---|---|---|
| Update only after verifying the PR and branch on GitHub | | | | | | |

When a task is closed, keep the historical record in the Checkpoint Log; remove it from this active table only after its closure status is verified.

## Checkpoint log

Append a new dated entry for each meaningful milestone. Include links to commits, PRs, test results, requirements, and relevant files.

### Entry template

#### YYYY-MM-DD — CHECKPOINT-ID — short title

- Status: OPEN / CLOSED / BLOCKED / SUPERSEDED
- Confidence: CONFIRMED / INFERRED / UNVERIFIED
- Goal / requirement:
- Repository and branch:
- Base SHA:
- Resulting SHA:
- PR:
- Files changed:
- What was changed:
- What was verified, and how:
- CI status for the exact resulting SHA:
- User-side tests still required:
- Decisions made and rationale:
- Known limitations / risks:
- Next exact action:
- Supersedes / superseded by:

## Branch and PR inventory rules

1. At the start of a session, query GitHub for the current `main` SHA, relevant open PRs, their head/base branches, exact SHAs, and CI status.
2. Search the repository's branches and open PRs before creating a new branch. Reuse an existing task branch only if its purpose, base, and current contents match the task.
3. Never rely on remembered branch names, chat summaries, old ZIP packages, or a PR number without rechecking the live GitHub state.
4. Before editing, identify the latest verified checkpoint and compare it with the actual repository state.
5. If two branches contain useful work, do not guess which one is canonical. Compare their commits and diffs, identify dependencies, and preserve both until the intended integration path is clear.
6. Never merge, close, retarget, force-push, or discard useful work without explicit user authorization.
7. When a PR is merged or closed, verify the resulting state and record the merge/close commit or reason. Do not treat a draft PR as integrated into `main`.

## Session start checklist

- [ ] Read this file and `00_PROJECT/GITHUB_DEVELOPMENT_PROTOCOL.md`.
- [ ] Fetch live `main` SHA from GitHub.
- [ ] Identify relevant open PRs and search branches before choosing a work branch.
- [ ] Read the last checkpoint and confirm it still matches GitHub.
- [ ] Check whether the latest checkpoint's CI and required manual tests are complete.
- [ ] State the intended task branch, base SHA, target outcome, and acceptance criteria before writing.

## Session end / handoff checklist

- [ ] Record the exact branch, PR, base SHA, and new head SHA.
- [ ] Record the files changed and the purpose of each change.
- [ ] Record confirmed results separately from assumptions.
- [ ] Record CI status for the exact new SHA and any missing/manual tests.
- [ ] Record the next specific action and any blocker.
- [ ] Commit the register update in the task branch / documentation PR.
- [ ] Do not call a checkpoint CLOSED until all required acceptance evidence exists.

## Recovery when the previous chat or branch is hard to find

1. Stop code edits.
2. Fetch current `main`, list/search open PRs, and search branches.
3. Read this register and inspect each listed branch/PR's actual head SHA.
4. Compare candidate branches against their recorded base and the relevant files.
5. Recover useful work by explicit commit/diff references; do not cherry-pick or overwrite based on branch names alone.
6. If the register is stale or contradicts GitHub, trust the live repository for current Git state, preserve the register history, and append a correction with the evidence.
7. Resume only after the active branch and latest confirmed checkpoint are unambiguous.

## Initialisation note

This register is being introduced as a control mechanism. Until its first verified state entry is populated from live GitHub data, it must not be treated as a complete inventory of all existing branches, PRs, or checkpoints.
