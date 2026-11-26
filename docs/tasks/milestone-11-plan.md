# Milestone 11: slow agreement implementation plan

> For agentic workers: use superpowers:subagent-driven-development and control AGENTS.md.

Goal: implement C019–C022 after all M10 acceptance/review/exact-tree/CI gates pass.
Architecture: one serialized C++20 Replica, one authoritative instance map, explicit
owned events/effects, immediate self-delivery; simulator shares the same core.
Spec: pinned EPaxos*2511.02743v1 Section3 Figure3, docs/design.md,
docs/baseline-transitions.md and approved docs/commit-plan.md.

## Global constraints

Baseline n=5,f=2,e=1. Slow quorum3 distinct replicas includes real local reply.
No Section4 optimizations, recovery initiation, execution or durability in M11.
Current and accepted ballots remain distinct. Preserve initial command/deps.
Root owns Git. Every slice RED/GREEN TDD, independent review, exact staged Debug
and Release Werror with complete suites, no skips; three repair/re-review cycles
maximum per milestone. No branch publication rewrite or automatic merge.
Positive-ballot fixtures test guards without inventing recovery transitions.

## Task 1 — C019: frozen slow proposal

Files: protocol Replica/initial agreement and new slow agreement source/header as
needed, events/effect runner integration, unit/simulation agreement tests, notes.
Consumes C017 SlowPathReady frozen observation map. Produces one immutable
per-ballot proposal and self-first Accept broadcast automatically; no external
peer/client input needed after three responders and grace expiry.

- [ ] RED: real runner three responders expiry automatically broadcasts Accept;
  union includes every frozen response, fourth mismatch retained; late/fifth or
  duplicate handoff cannot mutate/rebroadcast; frozen command/ballot retained.
- [ ] Implement local continuation through explicit typed event/effect path,
  validating ownership/active slow selection and avoiding a second authoritative table. A forged handoff matching fast-sealed observations before self-Commit cannot authorize Accept; test this branch distinction.
- [ ] Keep Accept receipt unsupported until C020; no synthetic self-vote.
- [ ] Review, exact staged Debug/Release and commit
  `protocol: freeze slow-path dependency proposals`,2026-07-13T13:30:00-04:00.

## Task 2 — C020: exact Accept transitions

Files: protocol accept handler/helpers, Replica dispatch, unit tests, notes.
Consume Accept(ballot,id,payload,deps); produce one matching AcceptOK.

- [ ] RED exact guard current<=b && (current!=b || phase!=committed), including
  lower ballot rejection, equal committed exclusion, higher ballot same-value
  acceptance after committed, unknown instance materialization and allocator
  high-water for local IDs. Test current versus accepted separately.
- [ ] Preserve initial metadata; install candidate deps without conflict expansion;
  set bal/abal and accepted phase. Equal-ballot contradictions against an already accepted/frozen proposal or retained committed decision reject
  atomically; identical duplicate reply replay is documented transport refinement.
  Distinguish committed-value diagnostics from paper guard, no blanket exclusion. Legitimate Accept(0) replaces preaccepted expanded dependencies with selected-Q union, even when smaller; test this explicitly and never compare preaccepted deps as an immutable accepted proposal.
- [ ] Test timer/collector supersession and frozen local proposal ownership.
- [ ] Review, exact staged Debug/Release and commit
  `protocol: implement ballot-guarded accept transitions`,2026-07-13T15:45:00-04:00.

## Task 3 — C021: distinct slow quorum commit

Files: slow collector, Replica/common Commit path, unit/simulation tests, notes.
Consumes exact-ballot AcceptOK (no value fields); proposal immutability supplies
value identity. Produce self-first common Commit once at third distinct vote.

- [ ] RED: real self vote counts once; two distinct/four duplicate packets cannot
  commit; third commits without fourth/fifth; stale/higher ballot OKs and unknown
  or unowned collectors cannot count; late replies cannot alter or resend value.
- [ ] Match current ballot and accepted phase, seal before effect execution,
  reuse common exact-equality Commit handler. Preserve initial metadata.
- [ ] Review, exact staged Debug/Release and commit
  `protocol: commit slow-path quorum decisions`,2026-07-13T18:15:00-04:00.

## Task 4 — C022: failure and replay evidence

Files: simulation slow-agreement tests and boundary docs.

- [ ] Real five-node disagreement and three-survivor commit; two-survivor
  nondecision; duplicated/reordered Accept and OK, stale/superseded ballots,
  Commit arriving before Accept; coordinator crash before Accept/during collection/
  after self-commit; captured trace serialize/parse reconstruction and replay.
- [ ] Assert actual branch evidence and every observed chosen value, no invented
  recovery or liveness after permanent coordinator loss. Validate new test oracle
  against a meaningful isolated mutant if behavior already implemented.
- [ ] Independent task and whole-M11 review; exact staged Debug/Release; commit
  `test: cover interrupted slow agreement and stale commits`,2026-07-16T10:10:00-04:00.
- [ ] Push only feature branch, open stacked PR against verified M10 branch;
  green CI, exact base/head anchors and persisted evidence before M12.

## Preflight interfaces and review focus

C017 handoff -> C019: includes all observations through selection, |Q|>=3;
exactly3 is survivor case, not a blanket Q size restriction. C019 computes union
once; Accept has no initDep extra wire field. C019 -> C020 does not fabricate
local acceptance before real self-delivery. C020 -> C021 guards proposal and
ballot identity, distinct senders, accepted phase and once-only selection.
Higher ballot Accept may revisit accepted phase after committed; do not infer
phase monotonicity. Preserve immutable decision evidence where required.
C022 owns full failure/replay fixtures, not a general M12 observer or proof.
Read-only preflight/root/c017_spec on2026-11-25 found no paper blocker; detailed
preparation in M10 workspace m11-spec-preparation.md. M10 gate remains prerequisite.
