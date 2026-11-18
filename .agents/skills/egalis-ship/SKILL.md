---
name: egalis-ship
description: Prepare coherent Egalis feature commits and a reviewed GitHub PR, monitor CI and preserve historical main without merging.
---

Read AGENTS.md and docs/engineering-workflow.md. Confirm task authorization,
expected base, clean/unrelated changes, remote identity and gh auth status.
Reuse the task's branch/worktree and PR when present. Setup/control PRs target
checkpoint/milestone-9; protocol feature PRs target main and must not inherit
October/November control commits. Do not create duplicate PRs.

Before each commit require independent review and egalis-verify of the exact
staged tree. Inspect git diff --cached, artifacts/secrets and write-tree; stage
explicit paths. Root commits coherent changes, approved historical metadata only
for scheduled slices, actual timestamps for setup/control changes. Verify hash,
parent, identity, timestamps and tree afterward; never amend published commits.

Push only the exact intended branch: git push -u origin HEAD:refs/heads/<branch>.
No --force, --all, tags or main push. Create/update a PR with explicit base/head,
scope, actual-date evidence, review findings, limitations and decisions. Use
.github/pull_request_template.md; do not describe agent review as human approval.

Monitor gh pr checks (bounded waits, inspect logs with gh run view --log-failed),
and review threads. Up to three repair/re-review cycles total per milestone;
routine repairs need no new user prompt. On green checks and resolved findings,
report ready for user review; never merge. Pending/blocked checks remain explicit
and the PR stays draft until verification/review is complete. Persist PR URL,
commit IDs and remaining blockers in task state. Network/sandbox denial calls for
narrow permission escalation, never bypass or token exposure.
