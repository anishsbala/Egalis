# M10 session timing and workflow observations

Read-only analysis. No product, workflow, history or instruction edits. Evidence is streamed JSONL; source files measured first (original 14,123,607 bytes; resumed 2,444,564 bytes at inspection). Lines below are physical JSONL lines. Timestamp windows measure elapsed wall time, not CPU time, model activity or billable usage. Root and child spans overlap and must not be added. Build/test evidence is analyzed separately in gate-timing.json.

## Provenance and starting request

O = C:/Users/anish/.codex/sessions/2026/10/15/rollout-2026-10-15T08-48-39-01a13f9b-85a7-7d52-8050-6e35d9d8af91.jsonl
R = C:/Users/anish/.codex/sessions/2026/11/25/rollout-2026-11-25T17-30-15-01a214d4-bb06-78c1-83a1-860b35ec5cf3.jsonl

Verified M10 authorization O:2937, 2026-11-23T23:16:52.449Z: “How about this, for this Egalis project, I want to complete milestone 10 and then continue so forth, completing each milestone sequentially until it is complete”. The filename dates creation of a long-lived root; it does not date this request. O:2949 accepts scope and O:2964 spawns M10 spec work. O:2920 is a preceding hypothetical, not the operative starting request. O:2333 explicitly forbids starting M10 while publishing M9, so October activity is excluded. Guardian sessions in inventory have source.subagent.other=guardian and are excluded as task execution. November 25 same-root-metadata copies are forked histories, not independent root runs; adding their copied prompts would double-count. November 23 children are validated by metadata parent_thread_id and agent_path; no unrelated project session is counted.

## Measured windows and interruptions

| Window | Seconds | Interpretation/evidence |
|---|---:|---|
| Original authorization to user abort | 10769.109 (2h59m29s) | O:2937 to O:3864; only C015/C016 complete, M10 incomplete |
| Original long exec timestamp window | 6390.359 (106m30s) | O:3541 23:44:30.180 to O:3544 next day01:31:00.539 |
| Original explicitly aborted final exec | 1271.027 (21m11s) | O:3861 01:55:10.531 to O:3864 02:16:21.558; output explicitly aborted by user |
| Between original abort and resume | 159235.902 (44h13m56s) | O:3864 to R:13; excluded from work |
| Resume request to M10 ready/CI green | 11422.425 (3h10m22s) | R:13 22:30:17.460 to R:845 next day01:40:39.885 |
| Resume 50-second sleep timestamp window | 9673.186 (161m13s) | R:657 22:52:52.274 to R:660 01:34:05.460; tool reports9673.1266seconds |

O:3541 performs sequential task-brief generation using the Superpowers Bash helper (with scoped escalation) and a write_stdin poll of an existing verification process (yield1000). O:3544 reports a running exec cell and Wall time31.0seconds, inconsistent with the106m30s record interval. This is not evidence that a build consumed106minutes, nor a redundant test invocation. Approval latency, session suspension and host pause cannot be separated. O:3861 applies control-state documentation and calls git status; the21m11s ends in an explicit user abort, not a completed gate.

R:657 requests50000ms, yet returns9673seconds later. R:702 reports an unchanged baseline type-test timeout during a long host pause; R:725 explicitly calls host suspension plausible but unproven. A sensitivity calculation removing only this sleep's excess beyond50s leaves1799.239seconds (29m59s) of resumed elapsed time. Removing the entire original long exec plus final aborted exec leaves3107.723seconds (51m48s). Neither remainder is measured active time or a promised optimized runtime. No OS suspend/resume or approval-open/close telemetry exists here. The long discontinuities dominate wall-clock totals; optimizing ordinary review cannot recover them automatically.

## Research, implementation, review and handoffs

| Span | Elapsed | Evidence |
|---|---:|---|
| M10 paper preflight from authorization to announced no blocker | 226.127s | O:2937 to O:3028 |
| C015 implement child session |1156.520s | inventory c015_implement child:1 to342; includes permission trouble |
| C015 reviewer child session |92.435s | inventory c015_review child:1 to88 |
| C015 review spawn to root passes review announcement |105.035s | O:3478 to3526 |
| C015 review announcement to commit announcement |6531.936s | O:3526 to3574; includes unknown6390s exec interval |
| C016 implement child session |558.077s | inventory c016_implement child:1 to249 |
| C016 reviewer child session |145.498s | inventory c016_review child:1 to112 |
| C016 commit announcement to C017 dispatch |14.616s | O:3837 to3838 |
| Resume recovery before C017 dispatch |36.056s | R:13 to45 |
| C017 dispatch to review dispatch |570.146s | R:45 to314 |
| C017 review dispatch to review-pass announcement |80.148s | R:314 to381 |
| C017 verify dispatch to commit announcement |227.665s | R:320 to434 |
| C017 commit announcement to C018 dispatch |7.625s | R:434 to436 |
| C018 dispatch to whole-M10 review dispatch |328.300s | R:436 to562 |
| Whole-M10 review dispatch to pass announcement |55.499s | R:562 to615 |
| C018 retry pass announcement to pushed commit announcement |46.153s | R:767 to802 |
| C018 pushed announcement to PR-open announcement |40.928s | R:802 to824 |
| PR-open announcement to CI-green announcement |56.659s | R:824 to845 |

Review and exact-tree verification overlap: C017 review spawn22:40:23.662, verification22:40:31.430 (R:314/320); C018 review22:49:55.020, verification22:50:02.094 (R:562/568). These stages are already parallel. Child session windows include reading, tools and waiting; final events may precede root consumption. No active-time summation or subtracting all waits is justified. The original spec child spans919.306s but is reused with followups; it is not15minutes of serial preflight. Original C017 child has only88.048s recorded before interruption; R:38 confirms clean C016 checkout and no surviving C017 source, so resume repeats assignment/context recovery, not proven reimplementation of completed work.

## Skill timeline, plan adherence and repeated work

Original root skill reads appear at O:2941 (using-superpowers, brainstorming, writing-plans, using-git-worktrees), O:2957 (Egalis milestone), O:2972/2979 (subagent development, TDD, cavecrew, writing-skills, creator), O:3126/3136/3169/3178 (Egalis verify/ship and scope-rule updates), O:3557 (verification and review skills). Original normalized egalis-ship path occurs5times in root call text; egalis-next-milestone4times. Counts identify text references, not guaranteed complete file reads or distinct useful invocations. R:17/30/59/108/598 read relevant workflow, verification, review/planning skills mostly once per path. Avoid future repeated read-only instruction loading within unchanged skill revision; do not infer savings from these counts.

O:3217 records independent scope review finding two entry points still saying stop after one milestone; correction was required to honor newly authorized sequential scope. It is setup/control work inside the reported original M10 session and should be reported separately from protocol implementation. Original spawn calls O:2964/3055/3072/3478/3575/3757/3838 use default agents with model/task-name choices, whereas resumed R:39/45/314/320/436/562/568 explicitly use egalis_spec/implement/review/verify. This confirms improved role selection on resume; task names alone do not prove custom role loading in the original.

## Stumbles, quality and request conflicts

The clearest avoidable orchestration issue is writable-path preflight. O:3262 reports managed-worktree placement outside writable workspace and a stalled first test patch; O:3413 reports scoped patch route working6m45s later. O:3645 reports repeated edit permission interruptions; O:3651 moves the held worktree into C:/Coding Projects/Egalis-m10; O:3718 confirms direct tests now run. The first report to resolution report spans2h10m1s but overlaps the106m discontinuity, implementation and verification: it is not all permission waste. On resume R:300 staging fails, R:308 explains Git metadata outside writable roots, and R:309 repeats the same14-file stage/export command with scoped escalation. The repeat starts11.281s later and is demonstrably retry work, but elapsed approval overhead is not isolated. Preflight both checkout writeability and linked-worktree Git administrative directory before dispatch.

R:262 identifies a real simulator restart-order defect exposed by lifecycle tests; R:282 reports repair. R:518/541 report threshold-three mutant rejection. These are useful correctness work, not avoidable churn. R:702/725/767 preserve a timeout, investigate same-binary rerun, then rerun fresh strict Debug/Release without changing source or weakening timeout. The repair/verification loop is evidence of quality, not permission to skip future exact-tree gates. R:802 reports connector PR-creation permission failure followed by authenticated CLI; R:824 confirms publication. Prefer a known-capable authorized publication route after capability preflight, without claiming40seconds pure waste.

The original authorizes sequential continuation, and resumed execution advances to M11 only after M10 review, fresh exact-tree gates and CI (R:845). R:239 keeps C017 and C018 separate boundaries. No demonstrated request conflict in completing M10 was found. M11 events after R:845 are excluded from M10 timing. The later throughput-analysis request at R:1041 is also excluded.

## Wait/cost telemetry and safe opportunities

Original root matched87 call/result pairs in scope:56exec,7spawn,7send,6followup,5wait,3wait_agent,2interrupt,1list. Explicit wait/wait_agent timestamp windows total140.939s. Resumed matched110 pairs through M10-ready:55exec,34sleep,9send,7spawn,2wait_agent,2list,1followup. Sleep requests sum1555s; sleep plus wait_agent observed timestamp windows sum10654.104s, dominated by the anomalous sleep. They overlap agents' useful work and are not additive waste. Polling shorter would not make the builds finish sooner. Prefer event-driven bounded wait_agent/check completion notifications, retaining responsive user updates; measure completion-to-consumption delay before claiming gains.

Preserve independent review and fresh staged-tree Debug/Release. First improve writable-root/Git metadata preflight, reuse verified immutable context and generated task briefs, avoid repeated full-suite working-tree builds when focused TDD proof plus the mandatory exact-tree full suites suffice (gate analyst owns actual build counts), and record perphase timestamps with run/tree IDs. Keep review and verification parallel as already practiced. Collect dispatch/start/first-tool/ready/consumed timestamps, approval-open/close and host suspend/resume, monotonic process time alongside UTC, tool duration distinct from model thinking, and source-tree identity for every run. No token/cost attribution or savings percentage is supportable from ambiguous cumulative token records and absent billing semantics.

Gate cross-check: gate analyst confirms C015 verification.json completed2026-11-23T23:44:59.828Z, only29.648s after O:3541 call. Therefore the106m30s timestamp window is primarily delayed result/orchestration visibility, with cause unknown, rather than measured verification runtime. O:3564 to3567 is a separate precommit CTest rerun (no rebuild),22.245s wrapper; gate analyst reports4.03s Debug and3.70s Release suite runtime. Keep this actual repeat distinct from the long delivery gap.
