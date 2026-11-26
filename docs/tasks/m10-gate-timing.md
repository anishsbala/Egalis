# M10 local gate timing evidence

Measured on 2026-11-26; read-only analysis. No C++ build or source/history mutation performed.

## Scope and measurement limits

The six persisted gate envelopes below span multiple interrupted calendar sessions. They are elapsed wall-clock intervals, not CPU/active work time. They cannot be presented as an additive decomposition of a single three-hour session. The original runner did not record command start/end timestamps, so configure/build/Git/review/handoff time cannot be isolated exactly from verification.json. Missing timing is unknown, never zero.

## Measured persisted gates

| Run | Gate wall seconds | Debug CTest seconds | Release CTest seconds | Result |
| --- | ---: | ---: | ---: | --- |
| C015-verification | 175.737 | 3.86 | 4.87 | PASS |
| C016-verification | 145.817 | 3.12 | 3.0 | PASS |
| C017-verification | 159.822 | 3.56 | 3.42 | PASS |
| C018-verification | 9826.336 | 4.84 | 9666.76 | FAIL: Release timeout |
| C018-verification-retry1 | 150.336 | 3.8 | 3.46 | PASS |
| baseline | 173.336 | 2.81 | 2.21 | PASS |

Each wall duration is computed from startedUtc/completedUtc at lines2â€“3 of its verification.json. Machine-readable paths and extracted values are in gate-timing.json. Baseline122 tests/config; C015131; C016141; C017151; C018159. There are six paired fresh gates, hence12 configuration builds/CTest invocations; one pair is a retry after failed evidence.

## Totals and dominant outlier

- Measured sum of all six gate envelopes:10631.384seconds (177.190minutes). This includes host/scheduling/process wait and is not active engineering time.
- Measured five successful paired gates, including baseline:805.048seconds (13.417minutes). Successful full CTest runs cost2.21â€“4.87seconds/configuration.
- Failed C018 Release CTest block:9666.76seconds (161.113minutes); its single timed-out unchanged baseline test reports9661.82seconds. Other158 tests passed. The same failed gate Debug159/159 passed4.84seconds.
- Subtracting that anomalous CTest block from all gate envelopes leaves964.624seconds (16.077minutes). This is an arithmetic residual, not a proved active-time or counterfactual saving.
- Normal CTest time across the other11 configuration invocations sums38.95seconds. Removing ordinary whole-suite repeats can save only seconds in this repository at this scale; targeting is good iteration hygiene but does not explain hours.

Outlier evidence: C:/Coding Projects/Egalis-orchestration/build/m10-audit/C018-verification/evidence-20261125T225022220Z-c4cc0948/Release-tests.log:170 (timeout), :325 (total), and verification.json:2â€“3,:158 (wall interval/failure). Immediate focused same-binary reruns passed twice; fresh retry passed both full configurations. See C:/Coding Projects/Egalis-orchestration/build/m10-audit/C018-verification-retry1/focused-timeout-reproduction.log:1 and retry verification.json:2â€“3. Host suspension/process interruption is plausible inference from the contemporaneous long host wait, not established root cause. Do not promise161minutes of workflow optimization savings or suppress the failed run.

## Compilation versus testing

Ninja .ninja_log records command scheduler offsets. For each fresh configuration, max recorded end offset ranges41.701â€“117.883seconds; these are recorded compilation/link/discovery envelopes, not independently timed whole cmake command durations. The extraction is reproducible in analyze-gates.py and stored per configuration in gate-timing.json. Across all12 configurations these envelopes sum874.105seconds; do not add this to gate wall totals (already included). No cache-hit telemetry exists.

The baseline and all exact gates already use the installed pinned GoogleTest prefix, so repeated dependency downloads are not demonstrated. See C:/Coding Projects/Egalis-orchestration/build/m10-audit/gate.ps1:39â€“44 and C:/Coding Projects/Egalis-orchestration/.superpowers/sdd/milestone-10-plan/task-1-report.md:21. C017 reused the C016 working build explicitly (task-3-report.md:30â€“34); C018 created a fresh working build (task-4-report.md:50â€“63).

## Repeated full-suite work

C015 and C016 ledgers explicitly record precommit reruns after successful exact staged Debug/Release gates, with no intervening source repair recorded: C:/Coding Projects/Egalis-orchestration/.superpowers/sdd/milestone-10-plan/progress.md:51â€“53 and :87â€“89. Bounded transcript extraction now measures the repeats: C015 rerun suites4.03+3.70=7.73seconds; C0162.74+2.44=5.18seconds, total12.91seconds. Neither command rebuilds. C015 native command9.1366454seconds includes commit/identity checks; C016 native command8.7638459seconds also includes push/brief preparation, so these are not isolated Git times. Evidence C:/Users/anish/.codex/sessions/2026/10/15/rollout-2026-10-15T08-48-39-01a13f9b-85a7-7d52-8050-6e35d9d8af91.jsonl:3564,:3567,:3827,:3830. The helper commit.ps1 itself only checks HEAD/tree/staged diff then commits (C:/Coding Projects/Egalis-orchestration/build/m10-audit/commit.ps1:14â€“18,:28â€“34), so do not claim it automatically rebuilt.

Implementation full suites also preceded exact gates: C015 green complete3.72seconds (C:/Coding Projects/Egalis-m10/build/c015-working/green-complete.log:268); C016 final2.95seconds (C:/Coding Projects/Egalis-m10/build/c016-working/final-full-ctest.log:288); C017 final3.03seconds (same build/c017-final-full-ctest.log:308); C018 final3.59seconds (C:/Coding Projects/Egalis-m10/build/c018-working/final-full-ctest.log:324). These four final working full runs sum13.29seconds, excluding build cost. More working full runs are documented after failures/fixes; those cannot all be classified unnecessary.

C017 first working full discovered a genuine runner restart-order defect; its repair/retest was useful (task-3-report.md:19â€“24,:45â€“50). C016 full run found an obsolete assertion requiring scoped adaptation (task-2-report.md:7â€“8). C018 first/final full runs are documented at task-4-report.md:65â€“70. Do not remove required regression investigation merely because a prior filtered test passed.

## Other requested phases

| Phase | Evidence available | Timing classification |
| --- | --- | --- |
| Specification research | Preflight/spec report and ledger exist. | No command/phase clocks in local gate artifacts; unknown here. Parent transcript analyst owns interval reconstruction. |
| Implementation | TDD timestamps in task reports. C016 RED01:38:30 to finalGREEN01:41:13 is163seconds. | Measured milestone markers, not whole implementation effort; initial reading/patch construction excluded. |
| Handoffs/review | Agent identities/verdicts and ledger marker order. | No independent phase timing in gate artifacts; unknown here, not zero. |
| Git operations | Commit hashes/metadata/helper. | Approved reconstructed dates are not actual operation timings. Transcript tool-call durations needed. |
| CI | Root recorded final eight successful jobs in two runs45737119587/45737195291. | Network query unavailable in this sandbox; no independent timing total claimed. Root transcript has exact per-job34/29/43/43/57/47/51/70seconds; they overlap and must not be summed as wall time. |
| Retries | One failed exact pair + successful fresh pair, focused reruns. | Failed9826.336seconds; successful retry150.336seconds; focused reruns recorded, no whole phase start/end. |

## Evidence-backed optimization implications

1. Keep each protocol-safety slice fresh isolated exact-tree full Debug/Release with warnings-as-errors and independent review. The normal paired gate costs roughly2.4â€“2.9minutes measured here, not hours.
2. Reuse a completed exact-tree result if tree/toolchain/options/dependencies and required suite are unchanged; perform identity checks before commit, not unconditional rerun of identical tests/build. Never relabel a dirty working build as exact-tree evidence.
3. Use incremental implementation builds and filtered RED/GREEN tests. Avoid redundant final working full suite when the same handoff immediately enters exact full gates, while preserving broad tests needed to localize failures.
4. Preserve installed pinned dependency reuse. Compiler content-addressed cache can be evaluated with cache keys/provenance and fresh build directories, but no measured cache speedup exists yet. Do not reuse mutable CMake gate directories across different trees.
5. Capture command/phase start/end monotonic duration, parent/span identity, concurrency, failures, cache state and wait reason. Keep failed evidence. Record suspected host suspension separately from controllable execution time.
6. Parallel review with exact builds where snapshot frozen; avoid serial duplicate spec reading/handoffs. Local artifacts alone do not measure this saving. Root transcript analysis supplies measured interval bounds.

## Original three-hour session gap attribution

Original transcript authorization-to-last-marker span supplied by session analyst is2h59m29. A single exec call at transcript:3541 (2026-11-23T23:44:30.180Z) receives its recorded output at:3544 (2026-11-24T01:31:00.539Z), a6390.359second transcript interval (106m30.359s). The call prepares the C016 brief using an escalated bash task-brief helper, then polls the C015 gate. Its output reports `Wall time31.0seconds`, not106minutes; event completion metadata at:3543 is23:45:00.230Z. C015 gate verification.json:3 independently completed23:44:59.828Z. Therefore the106minute interval cannot be attributed to measured C++ compilation or tests. It is a measured transcript/tool-result delivery gap with cause unresolved (approval, host scheduling/suspension or recording delay are possibilities). Do not call it106minutes of active orchestration or claim the workflow changes can recover all of it.

The C015 repeated precommit CTest command happens after that gap at transcript:3564, not inside it. C016 repeats occur at:3827. This distinction prevents misleadingly attributing the long gap to duplicate clean builds.
