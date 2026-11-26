# Egalis engineering orchestration

## Authority and architecture

Read `docs/workflow-state.json`, `docs/roadmap.md`, `docs/commit-plan.md`,
`docs/design.md`, `docs/testing.md` and the relevant transition/component notes
before selecting work. The current plan is on the control/checkpoint branch,
not main's stale boundary documents. Verify progress against source, tests and
Git; do not mark slices complete from plan text alone.

EPaxos* **arXiv 2511.02743v1, Section 3, Figures 1/3/5** is authoritative;
original EPaxos is background only. Baseline n=5, f=2, e=1; no Section 4
optimizations. Preserve one serialized C++20 Replica core shared by simulation
and future servers, explicit events/effects and immediate self-delivery.
Keep ballots/accepted ballots, initial metadata/candidate/committed values
distinct. Never invent guards, recovery rules or blanket dependency monotonicity.
Duplicate replies are not distinct votes. Volatile resume is not durable restart.

## Autonomous task loop

Use `$egalis-next-milestone` for the next dependency-ready, unverified milestone.
The default scope is one milestone. When workflow-state.json records an explicit
user-authorized sequential range, complete its milestones in numerical dependency
order without requesting another prompt at each transition. Within that range,
advance only after acceptance, exact-tree Debug/Release verification, independent
review, resolved findings, green CI and persisted evidence. A verified milestone
is not a human-approved or merged milestone. Stop when the authorized range is
finished, the user pauses, a consequential decision needs approval, or bounded
repairs cannot resolve a failure. New user scope restrictions always take priority.
Delegate meaningful specification work to `egalis_spec`, implementation to
`egalis_implement`, independent review to `egalis_review`, and verification to
`egalis_verify`. Root owns scope, integration and Git. Parallelize independent
reads/review; serialize edits or assign separate worktrees and explicit ownership.
Do not spawn agents for trivial edits. If custom roles are unavailable, report
that and use explicitly instructed independent agents; do not claim role loading.

Establish paper-backed acceptance criteria before edits. Investigate and repair
routine build/test/CI failures without asking. Allow at most **three repair +
independent re-review cycles per milestone**; then persist evidence and escalate
unresolved failures. Do not lower checks, hide skips or equate green tests with
distributed-system safety. A new conversation resumes from persisted state/PR,
not a fresh speculative implementation.

## Verification and review

Use `$egalis-verify` and `scripts/verify.ps1`: fresh isolated Debug and Release
builds, warnings as errors, complete CTest suites (zero tests is failure), focused
edge/failure/replay coverage. No sleeps or timing-based assertions. Inspect exact
staged source exports before each engineering commit. Logs are ignored build
artifacts; record actual UTC dates, revision, dirty paths, counts and failures in
the task record/PR. Never fabricate runs, benchmarks, proofs or production claims.
An independent reviewer must inspect actual diff and relevant callers/spec,
report severity + file/line + reproduction/missing test, and recheck fixes.
Implementation self-review is not independent review; reviewer cannot be the
author. Verification agent investigates failures separately from the implementer.

## Git and publication

Read `docs/engineering-workflow.md` for the control/engineering split. Preserve
C001–C014 and checkpoint 675f043 exactly. Never work on main directly. Use an
isolated `feature/mNN-description` branch from current main for the first protocol
milestone; subsequent authorized milestones may branch from their verified,
unmerged predecessor and open stacked PRs against that predecessor branch.
Persist exact base/tip hashes and PR dependencies; never count an unverified
predecessor as completed. If an upstream branch changes, reconcile and reverify
dependent work before advancing; do not rewrite published downstream commits.
setup/control branches start from checkpoint. Do not mix control ancestry into
historical engineering PRs. Root may create relevant commits, push only its
feature/setup branch, open/update a PR, monitor checks and fix scoped failures.
Never merge, force-push, rebase published history, delete important branches or
change the calendar. Historical dates/messages come from the approved plan;
actual verification dates remain accurate in control/task evidence. Setup commits
use actual dates, are outside C001–C112, and target checkpoint, not main.
Use explicit staging paths; preserve unrelated edits. Never commit build trees,
downloaded dependencies, backups, credentials or environment dumps. No secrets
in logs or PRs. Inspect staged diff and metadata, and preserve resumable state.

## Decisions versus routine autonomy

Proceed with scoped implementation, tests, refactors needed for correctness,
local worktrees, review, commits, feature pushes and PR preparation. Ask the user
only for consequential protocol/spec ambiguities, architecture/dependency changes,
timeline/merge-policy implications, security/destructive actions, new authority,
or failures unresolved after the bounded loop. Missing permissions are not
permission to bypass the sandbox. User reviews architecture decisions and PRs;
never merge to main on their behalf.

## Review guidelines

Review against the pinned variant, not generic Paxos intuition. Prioritize
incorrect phase/ballot guards, duplicate voting, lost dependency knowledge,
proposal mutation, lifecycle leaks and replay divergence over stylistic issues.
Flag unsupported correctness claims and untested failure assumptions. Never
approve solely because unit tests pass.
