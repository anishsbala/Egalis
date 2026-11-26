# Workflow throughput improvement — actual start 2026-11-25 local

Authorized by the user's throughput requests and pasted follow-up; one existing
orchestrator remains responsible for gated sequential M10–M36 advancement.
Branch codex/workflow-throughput starts at controlc06539c, targets
setup/autonomous-workflow; engineering ancestry remains separate. No merge.

## Evidence before changes

See m10-session-timing.md and m10-gate-timing.md for measured values and exact
local evidence references. Original authorization-to-abort2h59m29s includes
106m30s delayed tool-result interval and21m11s user-aborted call. Resumed M10
through green CI3h10m22s includes161m13s return from requested50s sleep.
These are elapsed windows, not measured active time or optimization savings.
Five successful paired local gates including baseline total13m25s; ordinary
CTest2.21–4.87s/config. C015/C016 unchanged precommit test repeats cost12.91s
combined, with no rebuild. Fresh Ninja envelopes42–118s/config. Review and
verification already overlap. Dependencies already reused installed GTest.

Research/implementation/review/handoff marker spans and Git/CI limits are in the
session report; intervals overlap. Missing command clocks remain unknown.
No percentage speedup, CPU-time or billing claim follows from these records.

## Chosen changes and tradeoffs

Retain fresh isolated exact staged full Debug/Release Werror per protocol-safety
slice. Use targeted RED/GREEN and one persistent incremental implementation build
per source/toolchain/configuration; do not clean per edit or slice without cause.
Tests-only and documentation work require proportionate proof; full final
milestone exact-tree Debug/Release, appropriate sanitizers and independent
correctness review remain required before PR completion. Independently buildable
boundaries remain; no unverified milestone advancement. Remove redundant
unchanged working full suites and post-gate reruns, not required regressions.

Reuse pinned installed dependencies and optional compiler content cache. Every
strict gate still starts with fresh CMake/output directories; never share mutable
CMake caches across exports, compilers, configurations or sanitizer modes.
Cache hits accelerate compilation only, not tests or source identity checks.
Cold cache can add setup overhead; no speedup is measured yet.

Keep one spec acceptance artifact; reuse it until relevant spec/source changes.
Use bounded briefs and report links rather than whole transcript forks. Reuse
idle reviewer/verifier roles for scoped follow-up; avoid spawning trivial helpers.
Parallelize independent research and frozen-tree review/verification, serialize
coupled protocol edits. Explicitly assign ownership and preserve all work.
Use native PowerShell/Python helpers on Windows; no second Superpowers planner,
Bash workflow or nested coordinator. Preflight checkout and linked Git metadata
permissions before delegation; do not bypass sandbox policy.

Record command start/end/elapsed plus configuration/cache provenance in the
verifier. Provide a standard-library phase recorder with unique activity IDs,
parent linkage and overlap-aware summary for spec, implementation, handoff/wait,
build/test, review, debugging/repair, Git and CI. Counts describe observed spans,
not CPU work. Preserve failures/retries and clock anomalies; no time-based tests.

Superpowers installed skills supply TDD/debugging/review techniques only. Context7
exposes resolve-library-id/query-docs tools; use only for unfamiliar/versioned
third-party APIs, never protocol authority. Caveman installed instruction-based
concise reporting/presets; retain full invariants/tests/findings and normal prose
for persisted artifacts. No proxy/runtime enabled; no cost saving claimed.

## Implementation and acceptance

- [ ] Add command timing and optional compiler launcher to maintained verifier;
  focused failure-boundary tests first, preserve all existing strict checks.
- [ ] Add phase recorder/tests, explicit activity IDs and overlap-safe reports.
- [ ] Update orchestration instructions, roles and three reusable workflow skills;
  concise dashboard records current agents, hashes, PR stack, gates, blockers/time.
- [ ] Support local-only compiler cache without caching binaries as proof
  or relaxing any compiler/configuration/sanitizer/required job. Remote cache uploads were rejected by automatic approval review and are omitted.
- [ ] Independent review of actual diff; fix legitimate findings/re-review.
- [ ] Full workflow regression tests, exact control staged Debug/Release and CI
  including sanitizer matrix, then separate reviewed control PR, never merge.

## Concurrent engineering state

M10 PR2 verified-unmerged headae50e873, mainbase32e0c855, all8 CI passed.
At initial hold C019 was uncommitted; it is now committed/pushed764854c0 after independent review and exact Debug/Release166/166. C020 continues incrementally.
Report .superpowers/sdd/milestone-11-plan/task-1-report.md. Resume its review/gate
without restarting implementation. Keep orchestration edits separate from its
15 engineering paths. The frozen baseline verifier pinned the C019 gate to
controlc06539c while runner changes are under review. No mixed evidence.

## Expected impact

Measured guaranteed avoidable test duplicate was12.91seconds across C015/C016.
Incremental builds/cache/context reuse may save minutes per milestone; magnitude
unmeasured and cache-dependent. Long host/approval/result gaps are not proven
workflow execution and cannot be promised away. Extra timing records/caches add
small overhead and maintenance; strict isolated gates deliberately retain cost.
Compare future phase ledgers before changing policy further.

## Independent review — 26 November 2026

workflow_review found P2 missing immutable launcher identity: path alone cannot
distinguish replacement. Tooling author is adding a content digest and regression;
version remains explicitly unknown unless available. Scoped re-review required.
No other actionable findings in verifier, recorder, CI triggers or policy. Initial
exact tree7eb6f8ab gate was already running when finding arrived; it remains
initial-tree evidence, not final modified-tree proof. No remote cache uploads.
