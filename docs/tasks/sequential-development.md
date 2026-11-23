# Sequential Egalis development — authorized 23 November 2026

User requested milestone 10 followed by all remaining milestones sequentially
until the project roadmap is complete. This supersedes the earlier single-
milestone stopping point, not correctness, review, history or permission gates.

## Run contract

- Range: milestones 10–36, numerical order respecting every prerequisite.
- Per milestone: specification evidence, justified commit slices, focused and
  complete Debug/Release tests with warnings as errors, exact staged source
  verification, independent review, scoped repairs, coherent dated commits, PR,
  green CI and persisted evidence before automatic advancement.
- Maximum three repair/re-review cycles per milestone. Never conceal failure or
  claim formal safety from tests. Optional milestone35 optimizations require
  actual profiling; omission must be explicitly recorded rather than padded.
- Verified readiness is separate from user acceptance/merge. No automatic merges,
  force pushes, calendar changes, history rewrites or unrelated mutations.
- Use stacked feature branches/PRs when predecessors are verified but unmerged.
  Root holds orchestration instructions on the control checkout. Engineering
  branches never inherit actual-date setup/control commits. Upstream changes
  require downstream reconciliation and verification before more advancement.
- This is an active on-demand run, not a newly scheduled background service.
  Persist actual state at interruptions; never report all milestones finished
  merely because the range is authorized.

## Anchors

- Engineering main/C014: 32e0c855d96af7154872d9e27352d2c7d3219005.
- Original checkpoint: 675f0435fce834a9abec39b6a4476e5281b240ab.
- Orchestration setup: 8fa455aaf90a9fca45f8f8c3b522a52e32c11012 (PR1).
- Completed at authorization: milestones1–9 only; 122 existing C++ tests.
- Next slice: C015, then C016/C017/C018 (milestone10).

## Progress

Milestone10: specification preflight completed by independent m10_spec, no blocker.
Fresh managed-worktree Debug and Release each passed122/122 with warnings as
errors. C015 is assigned to c015_implement for TDD; no slice is complete yet.
The approved roadmap, commit plan and pinned EPaxos* v1
remain authoritative. Grace-policy choice will be documented before its handler;
recovery and execution are not part of milestone10.

Scope-entry review caught old single-milestone/always-main wording in delegated
skills. Aligned both entrypoints and workflow prose with the explicit sequential
range. Scenario validation must cover green-but-unmerged M10, failed CI and an
upstream change; no human approval or integrated completion is inferred from
verification. Independent reviewer: /root/sequential_scope_review. Re-review
passed all three scenarios with no blockers. Metadata validators pass for both
updated skills and all14 workflow regression tests pass (18.460s). Old ignored
PyYAML tooling was inaccessible, so the same pinned6.0.3 dependency was restored
in a fresh ignored build directory; no global dependency/config changes.
