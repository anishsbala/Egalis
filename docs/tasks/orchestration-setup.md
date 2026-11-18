# Orchestration setup — actual date 18 November 2026

## Scope and state

Setup only; milestone 10 is not started. Control PR base is
checkpoint/milestone-9 at 675f0435fce834a9abec39b6a4476e5281b240ab.
Head branch setup/autonomous-workflow lives in the isolated worktree
C:/Coding Projects/Egalis-orchestration. Original C:/Coding Projects/Egalis
remains on main at 32e0c855d96af7154872d9e27352d2c7d3219005.
All 14 engineering commits and the original checkpoint remain unchanged.

Accepted setup decisions:

- Keep actual-date orchestration ancestry on the control branch. Do not inject
  it into the reconstructed engineering sequence; first engineering integration
  still requires the user's fast-forward-versus-real-merge-date decision.
- Four standalone project agent definitions inherit model/permissions; no
  global changes, approval bypasses, new service credentials or scheduler.
- Research/review are read-only behavioral roles. Hard isolation needs a
  read-only parent session, because custom agents cannot override parent sandbox.
- At most three autonomous repair/re-review cycles per milestone; no auto-merge.
- Use one PowerShell verifier and real failure-path integration tests. Evidence
  must distinguish working trees from exported indexes and reject missing tests.

## Environment checked

Codex CLI 0.170.0 is installed and ChatGPT authentication is active. GitHub CLI
authenticates as anishsbala with ADMIN repository permission. Remote main and
checkpoint exactly match the anchors above. Initial sandbox network denial was
resolved by a narrow escalation; no token was replaced or printed. Neither
branch protection nor repository rulesets existed at initial inspection.

Read current official Codex manual on this date for AGENTS discovery, standalone
custom-agent TOML, repository skills, worktrees, permissions and hosted review.
Installed skills are bundled/system skills; no personal/global project agents
were found. Existing global model/configuration remains untouched. The current
session exposes four concurrent collaboration slots; project config limits
spawned agents to three, leaving the root to integrate.

## Acceptance and evidence ledger

- Specification audit (independent spec_audit agent): pinned baseline invariants,
  next milestone 10 / C015–C018, and control/main timeline separation confirmed.
- Skill creator quick_validate: all three project skills pass. PyYAML was
  installed into ignored build/workflow-python only to run that bundled tool.
- Verification runner has real failure-boundary integration coverage, not only
  configuration text matching. Full Debug/Release builds, staged export and
  independent review results are recorded below as executed.
- Fresh CLI discovery probe uses read-only mode and --strict-config; it reads
  AGENTS and all three skills. The current spawn tool lacks a named custom-role
  selector, so file discovery is not proof of native named dispatch. The supported
  fallback explicitly loads each role's file into a separately spawned agent.
  The native-selector limitation must remain visible, not be claimed fixed.
- Optional hosted @codex review is not assumed enabled by CLI login. Local
  independent review is the working default; hosted enablement remains optional.

Logs and exports stay under ignored build/. Repository task summaries/PRs hold
actual dates and results; no environment dumps, secret values or generated binaries
are committed. PR/CI/final integrity results will be appended after execution.

Independent review cycle 1 found repeated build-cache/sanitizer-state leakage.
Fixed with per-run unique build trees and explicit ON/OFF configuration; added
regressions for these and native build failure. Re-review confirmed fixes and
found evidence artifact uploads would now include nested generated builds.
Narrowed CI uploads to top-level log/JSON/XML evidence only. Runner coverage is
13/13 passing integration tests. Skill validators pass and all five TOML files
parse. Worktree C++ Debug/Release both passed 122/122 before the final export gate.

A normal persistent CLI session then successfully spawned all four independent
role-bound agents sequentially (/root/egalis_spec, /root/egalis_implement,
/root/egalis_review, /root/egalis_verify). Each read its role file. The earlier
ephemeral thread-store failure was avoided without relaxing the sandbox. Native
named dispatch remains unavailable; the working fallback is explicitly verified.
That review identified an equal-count/substituted-JUnit-identity gap. Review
cycle 2 fixed it with case-sensitive name-multiset comparison and a regression
including a case-only substitution. Final workflow tests pass 14/14 (27.023s).
No milestone 10 code was written during these probes or fixes.
