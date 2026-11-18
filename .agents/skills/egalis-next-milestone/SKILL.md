---
name: egalis-next-milestone
description: Complete one dependency-ready Egalis milestone through independent review and a PR, using persisted progress and the approved plan.
---

Read repository AGENTS.md and docs/engineering-workflow.md first. This skill
orchestrates one requested milestone, not the whole roadmap or a standing daemon.

1. Reconcile docs/workflow-state.json with main, checkpoint plan and open PRs.
   Resume existing work rather than duplicate it. Select the first incomplete
   milestone whose prerequisites are verified; do not infer completion from dates.
2. Delegate paper/architecture analysis to egalis_spec. Record acceptance criteria,
   invariants, commit slices and any consequential decision in the task record.
   Resolve ambiguities before implementing affected behavior.
3. Create/reuse an isolated feature worktree from main. Keep orchestration in
   the control checkout: explicitly give workers its AGENTS.md, skill paths and
   approved plan, since the historical worktree does not contain this setup.
4. Assign slices in dependency order to egalis_implement. Require focused tests
   and independently buildable boundaries, not quota-driven commits. Use
   egalis-verify and an independent egalis_review pass before each slice commit.
5. Root stages only relevant files, validates the staged export, commits with
   approved metadata and verifies parent/tree/date. Keep actual-date evidence in
   the control task record, not historical engineering commits. Repeat across
   the milestone's justified slices without requesting a prompt per slice.
6. Use egalis-ship, wait for CI, fix legitimate findings, and return one PR with
   decisions and residual risks. Stop after the milestone, never merge it.

Persist task status, branch/worktree, slice hashes, acceptance evidence, review
findings, cycles used and PR URL in docs/tasks/<task>.md on the control branch
and/or the PR. Maximum three repair/re-review cycles total; on exhaustion mark
blocked with exact evidence and request user direction. User approval is needed
for new architecture/timeline policy, not ordinary engineering choices.
