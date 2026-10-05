#!/usr/bin/env python3
"""BlueSky PRO external development orchestrator."""
from __future__ import annotations

import json
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

REPO = os.getenv("BS_REPO", "ss1736427-source/BlueSky-PRO-Knowledge")
BRANCH = os.getenv("BS_BRANCH", "main")
POLL_SECONDS = int(os.getenv("BS_POLL_SECONDS", "5"))
CI_GRACE_SECONDS = int(os.getenv("BS_CI_GRACE_SECONDS", "5"))
AGENT_CONTINUE_SECONDS = int(os.getenv("BS_AGENT_CONTINUE_SECONDS", "5"))
GH_TIMEOUT_SECONDS = int(os.getenv("BS_GH_TIMEOUT_SECONDS", "20"))
PROJECT_ROOT = Path(__file__).resolve().parents[2]
STATE_FILE = Path(os.getenv("BS_STATE_FILE", str(PROJECT_ROOT / ".bluesky_orchestrator_state.json")))
ADAPTER = Path(os.getenv("BS_AGENT_ADAPTER", Path(__file__).with_name("agent_adapter.ps1")))
CI_PATH_PREFIXES = ("04_SOFTWARE/PLANNING/",)
CI_WORKFLOW_PATH = ".github/workflows/planning-benchmark.yml"
GENERATED_SUFFIXES = {".pyc", ".pyo"}


def tracked_paths() -> set[str] | None:
    """Return Git-tracked paths so cleanup never creates a working-tree deletion."""
    result = git_run(["ls-files", "-z"])
    if result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip() or "git ls-files failed"
        print(f"CLEANUP WARNING: cannot determine tracked paths: {detail}", flush=True)
        return None
    return {p for p in result.stdout.split("\0") if p}


def cleanup_generated_artifacts():
    """Remove only untracked disposable Python runtime artifacts from the checkout."""
    root = PROJECT_ROOT
    tracked = tracked_paths()
    if tracked is None:
        return
    removed = 0
    for path in root.rglob("*"):
        if path.is_dir() or path.suffix.lower() not in GENERATED_SUFFIXES:
            continue
        try:
            relative = path.relative_to(root).as_posix()
        except ValueError:
            continue
        if relative in tracked:
            continue
        try:
            path.unlink()
            removed += 1
        except OSError as exc:
            print(f"CLEANUP WARNING: cannot remove {path}: {exc}", flush=True)
    for directory in sorted(root.rglob("__pycache__"), reverse=True):
        if not directory.is_dir():
            continue
        try:
            relative_dir = directory.relative_to(root).as_posix()
            tracked_inside = any(p.startswith(relative_dir + "/") for p in tracked)
            if not tracked_inside:
                shutil.rmtree(directory)
                removed += 1
                continue
            for child in directory.iterdir():
                if not child.is_file() or child.suffix.lower() not in GENERATED_SUFFIXES:
                    continue
                relative = child.relative_to(root).as_posix()
                if relative in tracked:
                    continue
                child.unlink()
                removed += 1
            if not any(directory.iterdir()):
                directory.rmdir()
        except OSError as exc:
            print(f"CLEANUP WARNING: cannot remove {directory}: {exc}", flush=True)
    if removed:
        print(f"CLEANUP: removed {removed} generated Python artifact(s)", flush=True)


def gh_get(path: str):
    """Read a GitHub API endpoint, using GITHUB_TOKEN when supplied and gh as fallback."""
    url = "https://api.github.com/" + path.lstrip("/")
    headers = {
        "Accept": "application/vnd.github+json",
        "User-Agent": "BlueSky-PRO-orchestrator",
    }
    token = os.getenv("GITHUB_TOKEN")
    if token:
        headers["Authorization"] = f"Bearer {token}"
    request = Request(url, headers=headers)
    try:
        with urlopen(request, timeout=GH_TIMEOUT_SECONDS) as response:
            return json.loads(response.read().decode("utf-8"))
    except (HTTPError, URLError, TimeoutError) as direct_error:
        try:
            result = subprocess.run(
                ["gh", "api", path, "--hostname", "github.com"],
                check=True,
                capture_output=True,
                text=True,
                encoding="utf-8",
                timeout=GH_TIMEOUT_SECONDS,
            )
        except FileNotFoundError as exc:
            raise RuntimeError(f"Direct GitHub API failed ({direct_error}); GitHub CLI is not installed or not in PATH") from exc
        except subprocess.TimeoutExpired as exc:
            raise RuntimeError(f"GitHub API failed and GitHub CLI timed out after {GH_TIMEOUT_SECONDS}s: {path}") from exc
        except subprocess.CalledProcessError as exc:
            detail = (exc.stderr or exc.stdout or "GitHub CLI request failed").strip()
            raise RuntimeError(f"Direct GitHub API failed ({direct_error}); GitHub CLI request failed: {detail}") from exc
        try:
            return json.loads(result.stdout)
        except json.JSONDecodeError as exc:
            raise RuntimeError("GitHub CLI returned invalid JSON") from exc


def load_state():
    if not STATE_FILE.exists():
        return {"last_main_sha": None, "verified_sha": None, "decision_required": False}
    return json.loads(STATE_FILE.read_text(encoding="utf-8"))


def save_state(state):
    tmp = STATE_FILE.with_suffix(".tmp")
    tmp.write_text(json.dumps(state, indent=2, ensure_ascii=False), encoding="utf-8")
    tmp.replace(STATE_FILE)


def main_commit():
    print("Checking GitHub main...", flush=True)
    return gh_get(f"repos/{REPO}/commits/{BRANCH}")


def workflow_required_for_commit(commit):
    changed = [f.get("filename", "") for f in commit.get("files", [])]
    return any(p.startswith(CI_PATH_PREFIXES) for p in changed) or CI_WORKFLOW_PATH in changed


def ci_state(sha):
    runs = gh_get(f"repos/{REPO}/actions/runs?head_sha={sha}&per_page=20").get("workflow_runs", [])
    runs = [r for r in runs if r.get("head_branch") == BRANCH and r.get("head_sha") == sha]
    if not runs:
        return "UNVERIFIED", "No CI run for this SHA"
    runs.sort(key=lambda r: r.get("created_at", ""), reverse=True)
    run = runs[0]
    if run.get("status") != "completed":
        return "RUNNING", f"{run.get('name', 'workflow')}: {run.get('status')}"
    if run.get("conclusion") == "success":
        return "PASS", f"{run.get('name', 'workflow')}: success"
    return "FAIL", f"{run.get('name', 'workflow')}: {run.get('conclusion') or 'unknown'}"


def git_run(args):
    return subprocess.run(["git", *args], cwd=PROJECT_ROOT, capture_output=True, text=True)


def local_head_sha():
    result = git_run(["rev-parse", "HEAD"])
    if result.returncode != 0:
        raise RuntimeError(f"Local Git HEAD check failed: {result.stderr.strip()}")
    return result.stdout.strip()


def sync_local_checkout(target_sha):
    """Fast-forward local checkout to the exact GitHub target SHA, preserving runtime files."""
    local_sha = local_head_sha()
    if local_sha == target_sha:
        return True

    status = git_run(["status", "--porcelain"]).stdout.splitlines()
    allowed_runtime = {".bluesky_orchestrator_state.json", ".obsidian/workspace.json"}
    protected = []
    for line in status:
        path = line[3:] if len(line) >= 4 else line
        if path not in allowed_runtime:
            protected.append(line)
    if protected:
        print("LOCAL SYNC BLOCKED: uncommitted user changes are present", flush=True)
        for line in protected:
            print(f"  {line}", flush=True)
        return False

    fetch = git_run(["fetch", "origin", BRANCH])
    if fetch.returncode != 0:
        print(f"LOCAL SYNC ERROR: {fetch.stderr.strip() or fetch.stdout.strip()}", flush=True)
        return False

    remote = git_run(["rev-parse", f"origin/{BRANCH}"])
    if remote.returncode != 0:
        print(f"LOCAL SYNC ERROR: cannot read origin/{BRANCH}: {remote.stderr.strip()}", flush=True)
        return False
    remote_sha = remote.stdout.strip()

    if remote_sha != target_sha:
        print(f"LOCAL SYNC BLOCKED: origin/{BRANCH} is {remote_sha}, expected GitHub target {target_sha}", flush=True)
        return False

    local_in_remote = git_run(["merge-base", "--is-ancestor", local_sha, remote_sha])
    if local_in_remote.returncode != 0:
        print("LOCAL SYNC BLOCKED: local HEAD is not an ancestor of origin/main", flush=True)
        return False

    stash = git_run(["stash", "push", "-m", "BlueSky PRO orchestrator runtime state", "--", ".bluesky_orchestrator_state.json", ".obsidian/workspace.json"])
    if stash.returncode != 0:
        print(f"LOCAL SYNC ERROR: cannot preserve runtime files: {stash.stderr.strip() or stash.stdout.strip()}", flush=True)
        return False

    merge = git_run(["merge", "--ff-only", remote_sha])
    if merge.returncode != 0:
        print(f"LOCAL SYNC ERROR: fast-forward failed: {merge.stderr.strip() or merge.stdout.strip()}", flush=True)
        if not stash.stdout.strip().lower().startswith("no local changes"):
            pop = git_run(["stash", "pop"])
            if pop.returncode != 0:
                print("LOCAL SYNC ERROR: runtime-state restore also failed; user intervention required", flush=True)
        return False

    if not stash.stdout.strip().lower().startswith("no local changes"):
        pop = git_run(["stash", "pop"])
        if pop.returncode != 0:
            print(f"LOCAL SYNC ERROR: runtime-state restore failed: {pop.stderr.strip() or pop.stdout.strip()}", flush=True)
            return False

    new_head = local_head_sha()
    if new_head != target_sha:
        print(f"LOCAL SYNC ERROR: local HEAD is {new_head}, expected {target_sha}", flush=True)
        return False

    print(f"LOCAL SYNC: {local_sha[:12]} -> {target_sha[:12]}", flush=True)
    return True


def invoke_agent(sha, reason):
    if not ADAPTER.exists():
        raise RuntimeError(f"Agent adapter not found: {ADAPTER}")
    env = os.environ.copy()
    env.update({
        "BS_CURRENT_SHA": sha,
        "BS_CI_REASON": reason,
        "BS_REPO": REPO,
        "BS_BRANCH": BRANCH,
        "BS_PROTOCOL": "00_PROJECT/GITHUB_DEVELOPMENT_PROTOCOL.md",
        "BS_WORKING_RULES": "00_PROJECT/BLUE_SKY_PRO_WORKING_RULES.md",
    })
    return subprocess.run(
        ["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(ADAPTER)],
        cwd=PROJECT_ROOT,
        env=env,
    ).returncode


def run_agent_and_record(state, sha, reason):
    before_sha = local_head_sha()
    rc = invoke_agent(sha, reason)
    after_sha = local_head_sha()
    if rc == 42:
        state["decision_required"] = True
        save_state(state)
        cleanup_generated_artifacts()
        print("STOP: agent requested user decision", flush=True)
        return 42, False
    if rc != 0:
        state["agent_error_sha"] = sha
        state["agent_error_code"] = rc
        save_state(state)
        cleanup_generated_artifacts()
        print(f"STOP: agent failed with exit code {rc} for {sha[:12]}; fix the agent environment and restart", flush=True)
        return rc, False
    print(f"AGENT completed for {sha[:12]}; continuing automatically", flush=True)
    cleanup_generated_artifacts()
    changed = after_sha != before_sha
    if not changed:
        print(f"STOP: agent completed without creating a new commit for {sha[:12]}", flush=True)
        return 0, False
    return 0, True


def loop():
    state = load_state()
    print(f"BlueSky PRO orchestrator: {REPO}@{BRANCH}", flush=True)
    print(f"Project root: {PROJECT_ROOT}", flush=True)
    print(f"Polling: {POLL_SECONDS}s; CI grace: {CI_GRACE_SECONDS}s; agent continuation: {AGENT_CONTINUE_SECONDS}s", flush=True)
    last_agent_sha = None
    last_agent_time = 0.0
    while True:
        try:
            commit = main_commit()
            sha = commit["sha"]
            is_new_sha = sha != state.get("last_main_sha")
            if is_new_sha:
                state["last_main_sha"] = sha
                state["verified_sha"] = None
                state["decision_required"] = False
                state.pop("agent_error_sha", None)
                state.pop("agent_error_code", None)
                state["ci_required"] = workflow_required_for_commit(commit)
                save_state(state)
                print(f"NEW MAIN SHA: {sha}", flush=True)
                print(f"CI REQUIRED: {state['ci_required']}", flush=True)
                last_agent_sha = None
                last_agent_time = 0.0
            elif state.get("decision_required"):
                print("STOP: user decision required for current SHA", flush=True)
                return 42

            if not sync_local_checkout(sha):
                print("STOP: local checkout requires user intervention", flush=True)
                return 42

            ci_required = bool(state.get("ci_required", True))
            if ci_required:
                status, detail = ci_state(sha)
                print(f"{sha[:12]} CI={status} ({detail})", flush=True)
                if status == "PASS" and sha != state.get("verified_sha"):
                    state["verified_sha"] = sha
                    save_state(state)
                    time.sleep(CI_GRACE_SECONDS)
                    last_agent_sha = sha
                    last_agent_time = time.monotonic()
                    rc, changed = run_agent_and_record(state, sha, "CI passed for the exact current main SHA; continue the next unambiguous technical step.")
                    if rc == 42 or rc != 0 or (rc == 0 and not changed):
                        return rc
                elif status == "FAIL":
                    print("STOP: current SHA has failing CI", flush=True)
                    return 1
                elif status == "PASS" and sha == state.get("verified_sha") and last_agent_sha == sha and time.monotonic() - last_agent_time >= AGENT_CONTINUE_SECONDS:
                    last_agent_time = time.monotonic()
                    rc, changed = run_agent_and_record(state, sha, "Continue the next unambiguous technical step. Do not wait for a new user command when no decision is required.")
                    if rc == 42 or rc != 0 or (rc == 0 and not changed):
                        return rc
            elif sha != state.get("verified_sha"):
                print(f"{sha[:12]} CI=NOT_REQUIRED (workflow path filters do not require planning CI for this commit)", flush=True)
                state["verified_sha"] = sha
                save_state(state)
                time.sleep(CI_GRACE_SECONDS)
                last_agent_sha = sha
                last_agent_time = time.monotonic()
                rc, changed = run_agent_and_record(state, sha, "No planning CI run is required for the exact current SHA by the workflow path filters; continue the next unambiguous technical step.")
                if rc == 42 or rc != 0 or (rc == 0 and not changed):
                    return rc
            elif last_agent_sha == sha and time.monotonic() - last_agent_time >= AGENT_CONTINUE_SECONDS:
                last_agent_time = time.monotonic()
                rc, changed = run_agent_and_record(state, sha, "Continue the next unambiguous technical step. Do not wait for a new user command when no decision is required.")
                if rc == 42 or rc != 0 or (rc == 0 and not changed):
                    return rc
            time.sleep(POLL_SECONDS)
        except (RuntimeError, KeyError, json.JSONDecodeError, subprocess.CalledProcessError) as exc:
            print(f"ORCHESTRATOR ERROR: {exc}", file=sys.stderr, flush=True)
            cleanup_generated_artifacts()
            time.sleep(POLL_SECONDS)


if __name__ == "__main__":
    cleanup_generated_artifacts()
    raise SystemExit(loop())
