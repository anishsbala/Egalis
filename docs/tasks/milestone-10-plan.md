# Milestone 10: baseline fast agreement implementation plan

> For agentic workers: use subagent-driven development and the control AGENTS.md.

Goal: implement C015–C018 of the approved commit plan, then automatically advance
within the newly authorized range only after every milestone gate passes.
Spec: docs/design.md, docs/baseline-transitions.md and pinned EPaxos* v1 Figure3.
Architecture: one serialized Replica owns instance/proposal state; effect runners
execute owned effects. No alternate simulator algorithm. C++20/CMake/GoogleTest.

## Global constraints

Baseline n=5, f=2, e=1; four distinct original observations for fast commit; three
for slow fallback. Ballot zero is initial; Commit receipt requires exact local
ballot equality. Preserve initial metadata and frozen duplicate reply cache.
No recovery, execution, durable state or TCP in M10. Root alone stages/commits.
Tests precede implementation; real failure/replay evidence, not invented results.
Every slice: full Debug/Release -Werror, exact staged export, independent review.

## Task 1 — C015: distinct initial observations

Files: protocol replica header/implementation, focused agreement collector source
if needed, tests/unit/agreement_test.cpp, CMakeLists.txt, boundary agreement notes.
Interfaces: consume PeerMessage/PreAcceptOk and the local submission's frozen
InstanceState. Produce per-submission sender-to-dependency observations through
const inspection, separate from InstanceState::dependencies (the local vote cache).

- [ ] Add failing real-core cases: real self-OK counts once; three copies from
  one sender count once; changed duplicate rejects atomically; wrong/unknown ID,
  unsubmitted instance and pending-self Initial phase cannot collect; observations
  for concurrent submissions remain isolated; four equal expanded sets do not
  equal the original proposal. No decision or effects at this slice.
- [ ] Run focused tests and preserve expected RED output; implement first-response
  collection guarded by local initial owner, current ballot zero and PreAccepted.
- [ ] Run complete suites, self-review, independent review, staged gate and commit:
  `protocol: collect distinct preaccept observations`, 2026-07-10T14:35:00-04:00.

## Task 2 — C016: four original votes and common Commit

Files: agreement collector/Replica, new agreement transition source as justified,
tests/unit/agreement_test.cpp, tests/simulation/agreement_test.cpp, CMake/notes.
Consume Task1 observations and immutable initial proposal; emit self-first
Commit(0,id,command,initial-deps), exactly once, to all five members.

- [ ] RED tests: three votes/four packets with duplicate never commit; fourth
  original distinct vote commits without fifth; equal expanded values cannot
  fast commit; mixed original/expanded observations must also never fast commit
  (distinguish all-of from any-of); late/fifth/repeated replies cannot send another proposal.
- [ ] RED Commit receipt tests: unknown Commit(0) materializes directly, exact
  ballot guard, duplicate idempotence, contradictory committed values reject,
  lagging receiver may replace expanded dep with chosen smaller dep, initial
  metadata retained, no client success/execution. Pure fixtures cover higher
  ballot guard directions without implementing future recovery. Materializing
  a locally owned ID through Commit must advance the same allocator high-water
  mark as PreAccept, so a later SubmitCommand cannot alias or stall on that ID.
- [ ] Adapt only obsolete unsupported/PreAccepted-only expectations in existing
  tests, retaining substantive invariants. Complete tests/review/staged gate.
- [ ] Commit `protocol: commit baseline fast-path decisions`,
  2026-07-10T18:05:00-04:00.

## Task 3 — C017: deterministic grace and frozen slow handoff

Files: Replica/initial agreement, events/effects/runner trace handling,
tests/unit/agreement_test.cpp, simulation agreement tests, grace-policy ADR.

Ruling: one positive logical tick starts once at the third matching observation;
any mismatch with >=3 selects slow immediately; fourth original match before
expiry selects fast. First serialized winner freezes Q/observations. No selected
mismatch is discarded to manufacture a matching subset. This is a deterministic
choice permitted by Figure3, not an added consensus rule.

Ruling: C017 freezes immutable Q plus emits one observable local slow-handoff
effect; it does NOT emit Accept or count as agreement. C019 computes the union
from that frozen Q and broadcasts Accept. Do not invent a peer fallback message.

- [ ] RED tests: third exact vote schedules once, duplicate cannot extend it;
  fourth wins before expiry; expiry/mismatch seals Q; fewer than3 cannot choose;
  unrelated/stale timers cannot fall back; late replies cannot change frozen Q.
- [ ] Specify and test retained-memory suspend/resume: old generations/tokens
  inert, pending eligible grace can rearm through explicit runner effects.
- [ ] Document lifetime/effect ownership and tie rules; full tests/review/gate.
- [ ] Commit `protocol: schedule deterministic fast-path fallback`,
  2026-07-13T09:25:00-04:00.

## Task 4 — C018: deterministic integration/failure evidence

Files: tests/simulation/agreement_test.cpp, focused tests if missing, notes.

- [ ] Exercise self+three original voters, nonempty initial dependencies,
  three-survivor slow handoff, two-survivor nondecision, mismatches arriving
  before quorum/at fourth, both timer/reply same-time insertion orders, duplicate
  delivery, delayed PreAccept after Commit, coordinator crash before/after self
  commit and replay. Assert branch evidence and all observed committed values,
  not generic final-state equality when delivery differs.
- [ ] Run all suites + independent whole-M10 review and exact staged gate.
- [ ] Commit `test: cover fast-path quorum and coordinator races`,
  2026-07-13T10:50:00-04:00. Push feature branch, open PR targeting main, await CI.
- [ ] Persist exact tips, hashes, real dates and gates; only then start M11
  on the verified M10 tip with a stacked PR base. Never merge automatically.

## Review focus and preflight conflict scan

| Tasks/interface | Obligation and resolution |
| --- | --- |
| 1/2 observations | Collect without deciding first; C016 owns threshold effects. |
| 1/3 Q vs dep | Collector never mutates receiver vote cache; Q frozen at selection. |
| 2/3 selection | One latch guards fast vs slow, timers/replies cannot choose twice. |
| 3/C019 slow scope | Freeze Q/effect in C017; actual union/Accept begins C019. |
| 2/4 old scenarios | Update changed phase expectations without deleting invariants. |
| 1 internal | Sender/ID/phase/local-ballot guards precede atomic observation update. |
| 2 internal | Unknown Commit(0) allowed; no fabricated PreAccept requirement. |
| 3 internal | Real grace token/purpose, lifecycle rearm and bounded delay tested. |
| 4 internal | Replay actual decisions; permanent coordinator loss is not recovery. |

Highest-risk inputs are conflicting sender duplicates, equal expanded but non-
original votes, Commit overtaking PreAccept, unrelated/stale grace firings, and
crash/restart generations. Each has an owning test task above. The existing
specification preflight is build/m10-spec-report.md (ignored); no blocking paper
ambiguity was found. This plan refines the previously approved M10 slices and
does not require another routine implementation prompt.
