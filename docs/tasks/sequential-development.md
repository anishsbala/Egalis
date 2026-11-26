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
- At authorization: next slice C015, then C016/C017/C018 (milestone10). Current readiness is recorded in the latest gate entry and workflow-state.json.

## Progress

Milestone10: specification preflight completed by independent m10_spec, no blocker.
Fresh managed-worktree Debug and Release each passed122/122 with warnings as
errors. C015 completed, independently reviewed and committed locally:
53a8f33d248fb8d66e77a4e9b379436e4fea5a11, parent C014, tree
45dc035db9a472a38fc46ed76c0473fde7b02a5f, author=committer
2026-07-10T14:35:00-04:00 (approved reconstructed date).
Exact staged Debug/Release each passed131/131 with warnings as errors and were
rerun successfully immediately before commit. Actual verification date is
2026-11-23; all122 prior cases retained, nine new cases. C015 performs collection
only, not consensus decisions. C016 completed, independently reviewed and committed:
6d8aa66178dfd5f1126b975e6866a58694c49325, parent53a8f33d,
treeff9c27d08fb19c2daaabbed2721a571468fcb68c, approved author=committer
2026-07-10T18:05:00-04:00. Exact staged Debug/Release each passed141/141,
with all131 prior cases retained. Actual UTC verification completed on2026-11-24
(user-local date2026-11-23). C017 assigned to c017_implement; C018 unimplemented.
Both commits are pushed on feature/m10-fast-agreement. C015 remote GCC/Clang
sanitizer/non-sanitizer CI passed; C016 CI pending at this entry. Milestone10 is
not complete and cannot advance to11 yet.
The approved roadmap, commit plan and pinned EPaxos* v1
remain authoritative. Grace-policy choice will be documented before its handler;
recovery and execution are not part of milestone10.

The app initially placed the managed engineering worktree outside the writable
workspace, causing repeated patch-approval stalls. With explicit scoped approval,
the held clean C015 worktree was moved using git worktree move to
C:/Coding Projects/Egalis-m10 and reattached in the app; the old attachment was
unlinked without deleting any checkout or branch. HEAD remained53a8f33d and all
files/history were preserved. Control remains C:/Coding Projects/Egalis-orchestration.
Old build caches refer to the previous source path and are retained as evidence,
not reused; C016 will configure a fresh working build. No permissions were widened.

Scope-entry review caught old single-milestone/always-main wording in delegated
skills. Aligned both entrypoints and workflow prose with the explicit sequential
range. Scenario validation must cover green-but-unmerged M10, failed CI and an
upstream change; no human approval or integrated completion is inferred from
verification. Independent reviewer: /root/sequential_scope_review. Re-review
passed all three scenarios with no blockers. Metadata validators pass for both
updated skills and all14 workflow regression tests pass (18.460s). Old ignored
PyYAML tooling was inaccessible, so the same pinned6.0.3 dependency was restored
in a fresh ignored build directory; no global dependency/config changes.

## Resume audit — 2026-11-25

Both checkouts inspected with git status and git log -n3. Engineering clean at
6d8aa66178dfd5f1126b975e6866a58694c49325; no C017 changes survived interruption.
Control has existing edits to commit-plan, milestone-10-plan, sequential-development,
and workflow-state; preserved. Ledger is control-only, absent from engineering.
Remote main32e0c855, checkpoint675f0435 and feature6d8aa661 match expected anchors.
C016 CI run45308320834 completed success; only open PR is setup PR1.
Native egalis_spec and egalis_implement roles dispatched for resumed Task3.
Independent spec audit c017_spec found no blocker: Figure3 allows |Q|>=3;
freeze every observed reply through selection, including fourth mismatch Q=4.
Exactly3 applies only to three-survivor progress. Initial root dispatch wording
was corrected before implementation. First serialized event selects; old tokens
and unrelated timer purposes inert; retained-memory resume rearms through effects.
One tick does not guarantee fast selection for all unequal bounded delays and
M10 makes no execution/2Delta claim. C019 must consume handoff automatically.
C017 implementation active: /root/c017_implement. No review/staged gate yet.
Repair cycles remain0/3. C018 and milestone11 remain unimplemented.

Task3: complete (commit6d8aa661..ca49a0b7, review clean).
C017 exact tree5394199fca976fde994e8e78f64dd115c5d365c0 passed fresh
Debug151/151 and Release151/151, Werror, no skips. Actual UTC2026-11-25
22:40:51–22:43:31; evidence build/m10-audit/C017-verification/
evidence-20261125T224051806Z-1b28ca83/verification.json. Independent reviewer
/root/c017_review PASS spec/quality, no findings; verifier/root/c017_verify.
Commitca49a0b742f1d977694dd1aac93784e0d9ba2261 has approved author/committer
2026-07-13T09:25:00-04:00, parent/tree/identity checked. Not yet pushed.
C018 active implementer/root/c018_implement; task-4-brief.md. M10 not complete.
Repair cycles0/3. No advancement to M11 until full milestone gates and green CI.

C017 pushed on2026-11-25 to feature/m10-fast-agreement; CI pending at push.

Task4: complete (commitca49a0b7..ae50e873, review clean).
C018 exact treef5c22009f96cb728b7e14bf61f0459e09035fc08 fresh retry Debug159/159,
Release159/159 Werror, zero failures/skips,151 prior retained. Actual UTC
2026-11-26T01:35:10–01:37:40Z (25 November local); evidence C018-verification-retry1/
evidence-20261126T013510350Z-c7269e63/verification.json. Original failed run kept;
unchanged bounded baseline test passed twice immediately, host pause plausible
but unproved. No source/check/timeout changes. Independent task+whole review
/root/m10_final_review PASS no findings; verifier/root/c018_verify.
Committed/pushed ae50e873313af131f7648a295bfe64569fa6c137, approved date
2026-07-13T10:50:00-04:00; parent/tree/identity checked. PR2 draft opened
https://github.com/anishsbala/Egalis/pull/2, basemain32e0c855, headae50e873.
Connector PR create403; authenticated CLI fallback succeeded. CI pending;
M10 not yet verified-unmerged and M11 implementation not started. Repair0/3.


## M10 verified-unmerged gate — 2026-11-26T01:41Z

PR2 https://github.com/anishsbala/Egalis/pull/2 ready for user review; no merge.
Head ae50e873313af131f7648a295bfe64569fa6c137; base main
32e0c855d96af7154872d9e27352d2c7d3219005. Checkpoint675f0435 unchanged.
All8 GCC/Clang sanitizer/non-sanitizer checks passed across push run45737119587
and PR run45737195291. Whole-M10 review clean, exact final treef5c22009 passed
Debug159/159 and Release159/159. Acceptance complete; repair cycles0/3 (one
preserved environment-timeout gate rerun, no product repair). Human approval and
integration remain pending. M11 may branch from exact verified tip, stacked on
feature/m10-fast-agreement. Approved decomposition docs/tasks/milestone-11-plan.md.

## C019 completion and workflow optimization — 2026-11-26

C019 committed764854c01c6aea087045e53bbf07f30e20961494, parentae50e873,
tree5c68ffde45c27c337852cfb9452ecae7865e43fa, approved author=committer
2026-07-13T13:30:00-04:00. Independent review/root/c019_review PASS no findings.
Exact frozen controlc06539c gate Debug166/166 Release166/166 Werror, no skips,
all159 prior retained; UTC01:55:09–01:58:09 elapsed180.087s. Independent audit
/root/audit_c019_gate confirmed current index/export blob identity and safe reuse
without duplicate runs. Evidence build/m11-audit/C019-verification/
evidence-20261126T015508993Z-5f37aeba/verification.json. C019 not yet pushed.
C020 implementation active/root/c020_implement with focused incremental tests;
M11 incomplete. Optional nonzero coordinator coverage carried to C022.

Workflow changes isolated on codex/workflow-throughput from controlc06539c.
Measured M10 evidence in m10-session-timing.md/m10-gate-timing.md; no fabricated
speedup claims. Tooling25/25 tests passes; independent process review/gates pending.
Automatic review rejected remote GitHub compiler-cache uploads; omitted, local
compiler-cache support only. No cache proxy, history rewrite or merge.
