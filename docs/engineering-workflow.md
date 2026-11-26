# Autonomous engineering workflow

## Throughput policy — authorized 25 November 2026

Use AGENTS.md's throughput policy: one orchestrator, one acceptance record and one
active plan. Superpowers supplies TDD/debugging/review techniques without another
planning/SDD process. Context7 is conditional third-party API documentation, never
protocol authority. Caveman may shorten reports but cannot discard invariants,
negatives, results or findings. No input-compression proxy is enabled.

Keep incremental implementation builds stable within source/toolchain/config;
focused RED/GREEN plus affected regressions are the default edit loop. Clean for
an identified cache/toolchain issue or the mandatory isolated exact gate. Every
protocol-safety slice retains full exact staged Debug/Release Werror and
independent review. Broaden working tests for concrete uncertainty; do not
automatically repeat the same full passing gate. Review and verification overlap
on a frozen snapshot. One final review can cover both slice and milestone.
Reuse pinned spec findings and bounded briefs instead of whole transcript forks.

Reuse installed pinned GoogleTest with -GTestPrefix. Optional -CompilerLauncher
supports a locally installed compiler content cache; each strict gate still
starts in a fresh CMake directory and executes all tests. No unsafe sloppiness,
shared mutable CMake output, or executable-as-proof reuse. Launcher path/content
digest is recorded; version and cache hits may be unknown. Cache misses build
normally. No cache speedup is yet measured.
Remote compiler-cache uploads are not enabled: automatic approval review rejected
that proposed external payload. Local compiler caching needs no remote upload.

Control CI runs all PRs, protected-branch pushes and manual dispatch; feature
pushes need a PR for remote gates, avoiding duplicate push+PR matrices. All
compiler/configuration/sanitizer/required jobs remain. This changes control CI
only; historical engineering branches retain their own CI until separately
reviewed integration. Never import control ancestry to obtain the new workflow.

Record phase/source/parent IDs with scripts/workflow_timing.py. The verifier records
command clocks automatically. Preserve failures, interruptions and unexplained
gaps; elapsed time is not CPU work. Do not sum overlapping phases as milestone
wall time. Keep docs/tasks/dashboard.md and the task ledger current at meaningful
boundaries. See docs/tasks/workflow-throughput.md and the two M10 timing reports
for measured results, missing telemetry and expected tradeoffs.

## Control plane versus engineering history

Use `checkpoint/milestone-9` as the persistent orchestration/control branch.
This setup PR targets it, not main. It contains current planning, project agents,
skills and actual-date task records. Keep `main` at C014 until a user-reviewed
engineering PR is integrated. Both published anchors remain immutable.

The first protocol milestone starts from `main` in an isolated worktree and a
`feature/m10-fast-agreement`-style branch. Authorized subsequent milestones can
start from verified unmerged predecessor tips with stacked PRs. The root stays in the control
checkout, reads its AGENTS.md/skills/plan, and passes their absolute paths and
acceptance criteria to workers. Historical engineering worktrees deliberately
do not inherit the October/November control commits or configuration. Run the
control checkout's verification script with the worktree's source directory.

Never merge/cherry-pick this setup or the October checkpoint into main to make
instructions discoverable. Start Codex in the control checkout instead. GitHub
PRs against main still use main's currently published CI until CI is explicitly
integrated there. Until then, also run this control branch's stronger local
verification and link its evidence in the PR; do not claim main already has the
new required gate.

### The timeline decision at integration

Historical feature slices retain approved author AND committer dates/messages
from the plan; actual implementation/verification dates live in control records
and PRs. Setup/control commits use real dates and are not schedule slices.
GitHub merge creates an actual-date merge commit; squash/rebase would change
slice identities. Neither is silently selected. Before the first engineering
integration, ask the user to choose either a reviewed fast-forward (preserving
strict reconstructed chronological ancestry) or explicitly accept real-date
merge commits coexisting with reconstructed slice dates. Never rewrite existing
published commits, change the plan to hide the issue, or merge automatically.

## One-instruction operation

After the setup PR is reviewed and integrated into the control branch:

```powershell
Set-Location 'C:\Coding Projects\Egalis-orchestration'
# If this worktree still owns setup/autonomous-workflow, start Codex here during
# review. After integration, use an available worktree of checkpoint/milestone-9.
codex 'Use $egalis-next-milestone to complete milestone 10 through a reviewed PR; do not merge.'
```

The root selects/resumes the dependency-ready milestone and performs the full
bounded loop in AGENTS.md. It does not need another user prompt per slice, commit,
test, branch, handoff or routine repair. This is an on-demand orchestrated session,
not a scheduled service that runs while the computer is asleep. No scheduler,
extra API key, third-party automation framework or global model change is needed.

On 23 November 2026 the user authorized automatic sequential advancement from
milestone10 through roadmap completion. That active range in workflow-state.json
overrides the one-milestone default above. Each transition still requires all
verification/review/CI gates. Subsequent milestones may use stacked PRs based on
their verified unmerged predecessors; no merge or human approval is implied. See
docs/tasks/sequential-development.md for the active ledger and stopping rules.

Keep integrated-main completion separate from verified-unmerged readiness. Record
exact PR head/base SHA, gate evidence, findings and acceptance-criteria outcomes
for each unmerged milestone. Re-fetch/compare refs on resume and before advancing.
If upstream changes, invalidate affected evidence and reconcile/reverify first.
Never resolve a published divergent stack with an unapproved merge/rewrite or
silently invent reconstructed timestamps for post-publication repairs.

The active project must be trusted to load project configuration. Managed policy
and current sandbox/approval settings remain authoritative; do not disable them
for convenience. Project config only sets a three-subagent concurrency budget.
Models and reasoning inherit the parent/user selection.

## Roles and boundaries

`.codex/agents/*.toml` defines `egalis_spec`, `egalis_implement`, `egalis_review`
and `egalis_verify` using the current standalone name/description/instructions
schema. Research and review are read-only roles **by instruction**, not separate
OS sandboxes: current custom agents inherit their parent's permissions and cannot
override them with sandbox_mode. For hard read-only enforcement, run a separate
Codex review/research parent session with `--sandbox read-only`. Do not describe
role instructions as a filesystem security boundary. Verification writes only
assigned build artifacts; only root manages Git. Do not concurrently build into
the same directory or let agents edit overlapping files.

Skills are in `.agents/skills/` and auto-discovered at session startup. Existing
sessions may need a new chat to see new custom agents/skills. Their frontmatter
selects next-milestone orchestration, verification or shipping. The verification
skill calls one maintained PowerShell runner, not duplicated command scripts.

In this installed client, discovery succeeds but the exposed spawn tool has no
native custom-role selector. Root therefore explicitly reads each named role
file and supplies it when delegating to an independent agent. That role-bound
fallback is not proof of native custom-role dispatch. A CLI --ephemeral probe
also failed with a missing rollout/thread error; use a normal persistent session
for delegated workflows and retain this limitation in setup evidence.

## Verification tooling

With CMake, CTest, Ninja and GCC/Clang on PATH:

```powershell
./scripts/verify.ps1 -SourceDir . -Compiler g++
python -m unittest discover -s tests/workflow -v
```

The runner configures Debug and Release with CMake's compiler warning-as-error
property, uses CTest inventory/JUnit, rejects zero/skipped tests, checks native
exit codes, and writes dated commands/results under ignored build/verify.
Installed GoogleTest can be selected with -GTestPrefix; otherwise the existing
pinned FetchContent declaration applies. -CMake, -CTest, -Ninja and -Compiler
support local installations without committing machine paths. Use a new
-BuildRoot for staged gates and independent verifier runs. CI runs both compiler
families, both configurations, ASan/UBSan and the workflow's failure regressions.
The `required` aggregator fails if a prerequisite is skipped, cancelled or fails.

Export only the intended index into a fresh ignored directory using
`git checkout-index --all --prefix=<absolute-export>/`. Pass -SourceDir <export>,
-SourceTree <git-write-tree> and -SourceRevision <parent-commit> to verification;
the export's enclosing Git checkout must not be mislabeled as the tested source.
All exports, evidence, downloaded dependencies and backups stay untracked.

## GitHub, review and stopping

Root may create coherent commits, push an explicitly named feature branch and
open/update its PR. It must never push main, force-push, merge or delete important
branches. Use the PR template and explicit base/head. Monitor required checks,
inspect failed logs, and fix legitimate failures for at most three total
repair/re-review cycles. Persist findings and return a draft PR on exhaustion;
green CI alone is not independent review or protocol safety evidence.

Independent local agent review is available without GitHub Codex integration.
Optional hosted review (`@codex review`) requires repository enablement and plan
access in Codex settings; CLI authentication alone does not establish it. Do not
add API secrets or assume hosted review is configured. A human maintainer reviews
architecture and integration; an agent report is not a GitHub human approval.
This sole-maintainer repo may need a second collaborator before mandatory one-
approval protection is practical (GitHub authors cannot approve their own PRs).
Do not impose a lockout or allow an agent to bypass required checks.

Keep task records under docs/tasks with actual dates, branch/worktree, slice
hashes, accepted invariants, exact verification/review evidence, cycles and PR URL.
Update workflow-state only from verified completion, not a successful push alone.

## Configuration sources checked for this setup

- [Subagents and standalone custom-agent schema](https://learn.chatgpt.com/docs/agent-configuration/subagents)
- [AGENTS.md discovery](https://learn.chatgpt.com/docs/agent-configuration/agents-md)
- [Skills discovery](https://learn.chatgpt.com/docs/skills-and-plugins)
- [Worktrees](https://learn.chatgpt.com/docs/environments/git-worktrees)
- [Configuration reference](https://learn.chatgpt.com/docs/config-file/config-reference)

Checked against the installed Codex CLI 0.170.0 and the current official manual
on 18 November 2026. No global instructions/configuration were modified.
