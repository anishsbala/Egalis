# Verification record

## Initial design and scaffold — 15 October 2026

- Inspected the new project target; no existing Egalis directory was present.
- No workspace-level AGENTS.md existed at C:/AGENTS.md or C:/Coding Projects/AGENTS.md.
- Read baseline commit/recovery/execution material and supporting recovery policy
  and invariant sections in arXiv 2511.02743v1; consulted original EPaxos background.
- Created only design documents, build configuration, a C++20/GoogleTest toolchain
  test, ignore rules, and a CI definition. There is no protocol implementation.
- Detected MSYS2 GCC 14.2.0 at C:/msys64/ucrt64/bin/g++.exe.
- CMake was unavailable on PATH and absent from inspected MSYS2, project virtual
  environment, and Visual Studio BuildTools locations. GoogleTest was not located.
- Attempted a workspace-local pip installation of CMake 3.31.6 and Ninja 1.11.1.3;
  Windows network access denied the package connection (WinError 10013).
- Retried dependency acquisition through approved escalated execution; installed
  CMake 3.31.6 and Ninja 1.11.1.3 under the ignored build/tooling directory.
- CMake configured successfully with GCC 14.2.0 and pinned GoogleTest revision
  b514bdc898e2951020cbdca1304b75f5950d1f59 (verified against the fetched checkout).
- Compiled the scaffold successfully; CTest passed 1/1 toolchain smoke tests.
  This is build-harness verification only. The GitHub CI definition has not run.
- Read existing global Git author settings without changing them: anishsbala /
  anishsbala@gmail.com. GitHub association of this email has not been verified.
- No Git repository initialization, commit, timestamp modification, remote setup,
  history rewrite, or push was performed.

Successful local commands (PowerShell, from the project directory):

```powershell
& './build/tooling/cmake/data/bin/cmake.exe' -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe '-DCMAKE_MAKE_PROGRAM=C:/Coding Projects/Egalis/build/tooling/bin/ninja.exe'
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
& './build/tooling/cmake/data/bin/cmake.exe' --build build/debug --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/debug --output-on-failure
```

CTest reported: 100% tests passed, 0 tests failed out of 1. Build tooling paths
are local environment details, not repository requirements. Before the first
commit, review the design and agree on author email and timestamp schedule.
The detailed baseline-only handler mapping is milestone 4, before handler code.

## Batch 1: Git and milestone 2 — 15 October 2026

Confirmed the target is C:/Coding Projects/Egalis and neither it nor its parent
was a Git worktree. Initialized local main with `git init --initial-branch=main`;
set repository-local user.name/user.email to anishsbala / anishsbala@gmail.com.
No remote, staging, commit, historical timestamp, or push. Verified ignore rules
with `git check-ignore` for build tooling, fetched dependencies, binaries and
temporary files. The authenticated GitHub connector profile matched the identity;
the authenticated CLI calendar returned zero contributions on all 36 proposed
dates in docs/commit-plan.md. Schedule approval remains pending.

Milestone 2 adds fixed-membership IDs, checked numerical instance ordering,
separate current/accepted ballots, ballot ownership and unique quorum counting.
Used the configure/build/CTest commands above. GCC 14.2.0 built successfully;
6/6 tests passed (five new foundation tests and the existing smoke test).
Reviewed bounds, encoding injectivity and quorum membership. Initial test-only
nodiscard warnings were corrected before preserving the milestone snapshot.

## Milestone 3 — deterministic KV semantics

Added reusable GET/PUT/CAS commands and results, a deterministic in-memory
application model, and conflict detection with distinct absent/Nop payloads.
The exhaustive test checks 1,476 commuting pair/state cases and both returned
results, not just final state. GCC build and CTest passed 10/10 tests (four new).
Commands: the same `cmake --build build/debug --parallel 2` and
`ctest --test-dir build/debug --output-on-failure` as above; CMake regenerated
successfully. Reviewed absent/empty/binary keys and values and failed CAS mutation.
Corrected a test-only dangling-else warning before saving the source snapshot.

## Milestone 4 — baseline transition and message contract

Read Figures 3/5 and Appendix C in the pinned paper. Added
docs/baseline-transitions.md with baseline exclusions, handler guards and field
mapping; all eleven baseline message alternatives and structural validators;
initial instance records and an exact versioned canonical trace representation.
Agreement and recovery transitions remain unimplemented. Reviewed highlighted
optimized-code exclusions, ValidateOK broadcast destinations, validated initial
states, and accepted-ballot separation. Build/CTest commands as above regenerated
CMake and passed 19/19 tests, with nine new message tests and no compiler warnings.

## Milestone 5 — scheduler and simulated transport

Added typed logical time/duration/event identity/sequence, a single-threaded ordered
event queue, cancellation, bounded runs, observation and terminal callback failure.
Added the production-independent typed Transport/Endpoint boundary and an owning,
nonmovable simulated network. Self-delivery is synchronous. All existing tests
and ten new scheduler/network tests passed (29/29) with GCC and CTest commands above.
Review found that self-delivery unnecessarily checked unused network-delay overflow;
removed that check on the self path and added a boundary regression before final
milestone verification. No protocol dispatcher or agreement code is implemented.

The final milestone 5 boundary passed 30/30 tests after the self-delivery overflow
regression. Preserved its source archive in build/milestones/05-scheduler.tar.

## Milestone 6 — faults and recorded replay

Resumed after the laptop interruption by inspecting files and checkpoint archives.
The previous combined network patch had failed validation and had not changed
network.hpp/network.cpp; random/trace source files had been created. Completed
the network changes using ordinary incremental patches.

Implemented scripted/probabilistic delays, loss, duplicate copies, directed and
group partitions; crash/restart destination generations; node-bound timers;
SplitMix64-v1 bounded draws; versioned ASCII trace serialization; recorded-choice
replay and terminal divergence checks. Added ADR 0001 with exact fault semantics,
ownership, replay requirements and limitations. Reviewed source and transition
mapping, including a missing structural check that Nop cannot be accepted at
ballot zero; added a 30-case recovery field-combination regression.

Build and CTest commands above passed 50/50 tests on the first full milestone 6
run, then 55/55 after the review regressions (no warnings). Coverage includes 32
seeded fault scenarios replaying complete event decisions and delivered messages;
changed message/configuration/schedule, unused trace records and invalid random
choices fail replay. Callback failures and duplicate-time overflow fail closed.
Initial milestone snapshots 2–5 are retained locally under ignored build/milestones.
No staged files, commits, dates applied, remote or push.

Local GCC does not locate ASan/UBSan runtime import libraries; sanitizer execution
has not been claimed. GitHub Actions has not run. No consensus correctness,
durable restart correctness, or production readiness claim follows from these tests.

A separate optimized GCC Release build with `-DCMAKE_CXX_FLAGS=-Werror` passed
55/55 tests. Reused the pinned GoogleTest checkout without another network fetch:

```powershell
& './build/tooling/cmake/data/bin/cmake.exe' -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe '-DCMAKE_MAKE_PROGRAM=C:/Coding Projects/Egalis/build/tooling/bin/ninja.exe' '-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=C:/Coding Projects/Egalis/build/debug/_deps/googletest-src' -DCMAKE_CXX_FLAGS=-Werror
& './build/tooling/cmake/data/bin/cmake.exe' --build build/release --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/release --output-on-failure
```

Added an opt-in Unix ASan/UBSan CMake configuration and defined GCC/Clang sanitizer
jobs in the existing CI workflow. These jobs are defined, not remotely executed.
Disabled unused GoogleMock builds. Added LF normalization for portable source diffs.
The final batch source snapshot is build/milestones/06-faults-replay.tar.

Final review enforced the scheduler observer contract with an explicit mutation
guard (preventing cancellation from invalidating the EventInfo under observation),
removed an unnecessary event-label copy, and added its regression test. Debug and
Release both built and passed 56/56 tests; Release retained `-Werror`. Inspected
the actual scheduler/network diff against the milestone 5 snapshot and checked
the complete milestone diff for whitespace errors. Refreshed milestone 6's archive
after this review. Git remains main, with an empty index, no commits and no remotes.
Milestone 7 is the next dependency-ready milestone, in batch 2; no blocking
architectural decision remains for the completed batch.

## Calendar and granularity planning revision — 15 October 2026

Superseded the unapproved older calendar with a 2026-only proposal. Queried
authenticated GitHub GraphQL for viewer anishsbala using the explicit inclusive
July 1–October 14 interval (`gh api graphql --input docs/contribution-calendar-query.json`).
The 106 daily records contain 69 contributions, 87 zero days, nine one-contribution
days and ten days with at least two contributions. Evidence and analysis are in
contribution-calendar-2026.json and contribution-calendar-analysis.md.

Revised commit-plan.md retains all 36 milestone dependencies and the six reviewed
boundaries. Proposed 112 meaningful commit slices on 30 observed zero days:
45 in July, 40 in August, 16 in September and 11 through October 14. Two October
index/performance commits are conditional; omitting them yields 110 commits and
25 additions in September/October combined. All timestamps remain unapproved,
intentionally reconstructed metadata. Old timelines inside recovery snapshots
are preserved artifacts, not instructions to commit in 2025.

This task changes planning/evidence documentation only. No new C++ test execution
or benchmark result is claimed. The previously recorded 56/56 Debug/Release
results remain the implementation verification; new future slices are unimplemented.
Existing code, tests, reviewed trees, alternate indexes, snapshots and validation
artifacts are preserved. Milestone 7 has not started; no commit, timestamp change,
reset, remote configuration or push is performed.

Planning audit passed: 112 complete ledger rows covering all 36 milestones;
chronological 2026 timestamps with the same local/UTC day; all original roadmap
prerequisites and the optional-M35 bypass; 30 observed zero-day targets; monthly
allocations 45/40/16/11. Verified 389 intact captured checksum records and separately
compared all 28 implementation/configuration/test files with the original validated
manifest. All five milestone archives match their preserved copies; the original
final-source backup matches its recorded SHA-256. One truncated terminal checksum
record was excluded rather than treated as evidence. The main index remains empty,
with no refs or remotes created. Audit helper/evidence live under ignored
build/planning-audit-2026; existing preparation artifacts remain unchanged.

## Approved six-commit history finalization — 15 October 2026

User approved the revised 2026 schedule. Created six local commits on main,
with exact approved author and committer dates and consistent identity
anishsbala <anishsbala@gmail.com>. Every commit message explicitly discloses
that its timestamps are reconstructed calendar metadata, not actual engineering dates.

Preserved the current working tree before reconstruction at
build/approved-history-2026/pre-commit-working-tree.tar, SHA-256
CC4B2B7658DDFB2DC0930FED3FF5E661141BABBDF36A57EC6716531A9D3A82A7.
All older recovery trees, snapshots, backups, review artifacts and alternate
indexes were preserved; 361 original artifact checksums were rechecked unchanged.

Reconciled each candidate's README, roadmap, commit plan and verification summary.
Removed obsolete 2025 schedules and October-dated planning/verification records
from the committed candidates. Boundary verification documents explicitly do not
represent historical test sessions. Current October evidence, full 112-slice plan
and this actual dated verification record remain in the working tree, uncommitted.
No implementation, test, CMake, CI or ignore-rule bytes differ from the six original
reviewed boundaries. Full tree hashes necessarily differ because documentation changed.

Commands actually executed from C:/Coding Projects/Egalis:

```powershell
& './build/approved-history-2026/validate.ps1'
& './build/approved-history-2026/commit.ps1'
& './build/approved-history-2026/final-verify.ps1'
git log --reverse --format='%h %p %aI %cI %s'
git diff --name-only HEAD -- include src tests CMakeLists.txt .github .gitignore .gitattributes
git status --short
git remote -v
```

validate.ps1 configured fresh isolated exports with GCC, Ninja, pinned installed
GoogleTest, Debug and -Werror. All six builds passed; CTest passed respectively
1/1, 6/6, 10/10, 19/19, 30/30 and 56/56. Per-boundary configure/build/test logs,
review diffs, indexes, exports and validated.json are under ignored
build/approved-history-2026. Auxiliary PowerShell runner issues were corrected
before successful execution; no failed helper attempt is counted as verification.

commit.ps1 used commit-tree for the exact validated trees, verified each parent,
identity, tree and timestamp, then established main and populated the main index
without checking out or overwriting working files. No existing history was rewritten.

| Milestone | Local commit | Author = committer timestamp |
| --- | --- | --- |
| 1 | e8b839db2e984c2338050538295a8ac4aae2c607 | 2026-07-01T09:35:00-04:00 |
| 2 | 1f5145eb7239bdf4063c582139c09b7bacdafaed | 2026-07-01T11:10:00-04:00 |
| 3 | 86059f0f6d4cffc58b0e32fc176ddf530dfffe98 | 2026-07-01T14:05:00-04:00 |
| 4 | 8675efee39b2aa7739658d1f211245ac872a023d | 2026-07-01T16:40:00-04:00 |
| 5 | 205c305089c7a8188c5671f9e50f2f6bcb4e91b3 | 2026-07-02T10:20:00-04:00 |
| 6 | cdd8ef48ce2dd692ef48aaa35daa730cc6ae7444 | 2026-07-02T11:35:00-04:00 |

final-verify.ps1 passed after commit creation: exactly six commits; first commit
has no parent, each subsequent commit has the preceding hash as its sole parent;
all committed trees equal the newly validated trees; original-to-new tree diffs
contain only the four approved documentation paths. All 28 live implementation,
configuration and test file hashes match the original final manifest. No generated
artifacts, binaries, backups or temporary files occur in any of the six trees.

Post-commit full Debug and Release builds and CTest runs both passed **56/56**,
with -Werror in both configurations. Final suite logs are logs/final-Debug.*.txt
and logs/final-Release.*.txt under the audit directory. Source was built from the
exact final committed export; the live implementation independently matches it.
No new tests or functionality were introduced during history finalization.

Main index matches HEAD; remaining differences are intentionally uncommitted
current documentation and calendar evidence only. No remote, push or milestone 7
implementation occurred. Batch 1 is complete; Batch 2 begins with milestone 7
(shared replica event dispatch), subject to the next session authorization.

## Milestone 7: shared replica dispatch — 15 October 2026

Implemented only C007–C009 in dependency order. The existing six commits remain
unchanged. Preserved the pre-existing working documentation in the ignored archive
build/batch2-audit/planning-before.tar before updating its current status.
No preaccept/agreement, dependency scan, recovery, durable storage or TCP work
was implemented. No remote or push was configured.

| Slice | Commit | Parent | Author = committer timestamp | Pre-commit Debug |
| --- | --- | --- | --- | --- |
| C007 | 3f51749ffe94f2c2afae48d8a2b2362188af90de | cdd8ef48ce2dd692ef48aaa35daa730cc6ae7444 | 2026-07-02T13:15:00-04:00 | 60/60 |
| C008 | 0d5b1a1a7d968e361d814733c37f7378c6062f81 | 3f51749ffe94f2c2afae48d8a2b2362188af90de | 2026-07-02T15:10:00-04:00 | 67/67 |
| C009 | ccc4c98b11d74385a04cf41e136dfd7d6c19f2a2 | 0d5b1a1a7d968e361d814733c37f7378c6062f81 | 2026-07-02T17:25:00-04:00 | 83/83 |

All author/committer identities are anishsbala <anishsbala@gmail.com>.
The commit helper verified the exact staged tree, parent, identity and dates
before proceeding. An alternate index staged only each slice's source/test/build
changes, dispatch documentation and a reconstructed boundary verification summary.
Existing uncommitted planning and calendar evidence were not staged. The main
index was synchronized without checking out or overwriting working documents.

New coverage: four event/effect contract tests, seven core dispatch tests, and
sixteen simulator/trace tests (27 additional cases total). Coverage includes owned
payloads, structural validation, authoritative initial records, local ID reuse,
canonical iteration, all eleven unsupported message alternatives, timer replacement
and ballot/token guards, unsolicited completions, volatile lifecycle isolation,
immediate self-effect ordering, bounded effect execution, duplicates, callback
lifetime, fail-closed errors and legacy trace compatibility. Sixteen seeds each
run the same five-replica fault scenario twice and replay serialized traces.
These exercise infrastructure, not agreement or crash-recovery correctness.

Actual commands used (PowerShell, project root):

```powershell
$env:PATH = 'C:/msys64/ucrt64/bin;' + $env:PATH
& './build/tooling/cmake/data/bin/cmake.exe' -S . -B build/batch2-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe '-DCMAKE_MAKE_PROGRAM=C:/Coding Projects/Egalis/build/tooling/bin/ninja.exe' '-DCMAKE_PREFIX_PATH=C:/Coding Projects/Egalis/build/history-preparation/a08bcd44afc749c398271347550bcace/dependency-prefix' -DCMAKE_CXX_FLAGS=-Werror
& './build/tooling/cmake/data/bin/cmake.exe' --build build/batch2-debug --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/batch2-debug --output-on-failure
& './build/batch2-audit/commit-slice.ps1' C007
& './build/batch2-audit/commit-slice.ps1' C008
& './build/batch2-audit/commit-slice.ps1' C009
& './build/tooling/cmake/data/bin/cmake.exe' -S . -B build/batch2-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe '-DCMAKE_MAKE_PROGRAM=C:/Coding Projects/Egalis/build/tooling/bin/ninja.exe' '-DCMAKE_PREFIX_PATH=C:/Coding Projects/Egalis/build/history-preparation/a08bcd44afc749c398271347550bcace/dependency-prefix' -DCMAKE_CXX_FLAGS=-Werror
& './build/tooling/cmake/data/bin/cmake.exe' --build build/batch2-release --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/batch2-release --output-on-failure
& './build/batch2-audit/amend-C009.ps1'
```

The initial post-commit Release build failed under GCC 14.2.0 -O3/-Werror with
a maybe-uninitialized diagnostic on a temporary effect-variant move. Replaced
temporary variant insertion with direct in-place construction of timer/cancel
alternatives. No warning suppression, reduced optimization or weakened tests.
Both configurations then passed; folded this integration correction into C009
with its unchanged approved dates and parent. The superseded C009 hash and failed
build log remain preserved locally, not disguised as successful verification.

After the final C009 amendment, rebuilt and reran the full Debug and Release
suites: **83/83 in each**, both with -Werror. Logs and commit metadata are under
ignored build/batch2-audit: final-Debug-build.txt, final-Debug-tests.txt,
final-Release-configure.txt, final-Release-build.txt, final-Release-tests.txt,
initial-Release-failure.txt and C007/C008/C009-commit.json. Committed source and
tests match the verified working implementation; only current documentation and
calendar evidence remain uncommitted. No generated artifacts were committed.

Architectural decisions are in replica-dispatch.md. The core has no transport or
clock dependency. Submissions register initial local records, never votes or KV
success. Unsupported handlers do not mutate protocol state. Suspend/resume is
volatile isolation only. Trace schema 2 adds core input/result/effect decisions;
schema 1 transport-only artifacts remain supported. The effect budget is a runner
diagnostic, not a protocol rule. Formal correctness and production readiness are
not claimed. Milestone 8 (known-conflict dependency computation) is next and unstarted.

## Milestone 8: known-conflict dependencies — 15 October 2026

Implemented and committed only C010–C011 in their approved dependency order.
Reviewed architecture, roadmap, testing, baseline transition mapping, dispatch
contract, protocol/KV state and simulator implementations. Consulted pinned
EPaxos* v1 Section 3.2 Figure 3 lines 3/9 and Appendix D Figure 9 Invariant 6.
Current cmd, not a union with initCmd, is the dependency predicate. Nop retains
conflict visibility; original metadata is preserved for future recovery predicates.
The proposal-expansion helper computes the line 9 union without implementing
the surrounding preaccept transition. No recovery or agreement rule was invented.

| Slice | Commit | Parent | Author = committer timestamp |
| --- | --- | --- | --- |
| C010 | b58c9d98aa83811cb88466eb1f556c063cbbbc71 | ccc4c98b11d74385a04cf41e136dfd7d6c19f2a2 | 2026-07-07T10:05:00-04:00 |
| C011 | 5be893c06070e69ac4304ee89ab3c07f4dc70db7 | b58c9d98aa83811cb88466eb1f556c063cbbbc71 | 2026-07-07T13:40:00-04:00 |

Identity for both author and committer: anishsbala <anishsbala@gmail.com>.
Both commit messages disclose intentionally reconstructed calendar metadata.
No prior commit was rewritten. Existing uncommitted planning documents were
excluded from staging; boundary verification summaries were staged from separate
files, without importing actual October planning/verification events.

Before implementation preserved the entire non-generated working tree in
build/milestone8-audit/before-working-tree.tar, SHA-256
A40AA1C62522B9C0CE65F6E53B9B5EC16E6620344E3C228EE3383913F3B3E54E.
Saved the original nine-commit sequence in before-commits.txt. Rechecked all 361
previously recorded backup/review artifact hashes unchanged; no old artifact was
overwritten. New audit tools, exports and logs remain under ignored build/.

C010: eight additional GoogleTest cases; complete working Debug and Release
suites passed 91/91 each with -Werror. Exact staged tree
65fbe4bc8294a87b2d411d8c9f5e30ac0396141f independently configured, built and passed
91/91 in both configurations before committing. Tests cover unknown targets,
conflicts across operation types, exact ID exclusion, all phases/ballots, retained
committed commands, bottom/Nop, all 24 insertion orders, payload/command relation
equivalence and live read-only ownership. The scan copies no query payload and
uses the one authoritative instance map, with no index or pruning.

C011: six more unit cases and two simulator cases; full Debug and Release
suites passed 99/99 each with -Werror. Verified proposal union preserves unknown
and nonconflicting IDs; self dependencies fail without mutation; current Nop and
validated initial records remain visible; original command/dependency metadata,
ballots and timer guards survive queries; no transitive closure or unsupported
candidate installation occurs. Eight seeds each run a five-replica fault scenario
twice and replay serialized traces. All replica tables are checked before and
after faulted deliveries; read-only queries add no trace decisions. Existing
sixteen-seed core replay and transport fault suites remain part of every full run.

Initial C011 test compilation failed on an intentionally discarded nodiscard
result inside EXPECT_THROW and a shared CTAD declaration for differently sized
partition arrays. Explicit void casting and separate array declarations corrected
the test code without weakening flags or assertions. Failure logs were preserved
as C011-initial-Debug-failure.txt and C011-partition-fixture-build-failure.txt.
No failing attempt is represented as successful verification.

C011's final staged tree ab28ec073a4e0850435b5a395e398680d4e11bf2 was independently
built and tested in Debug and Release, 99/99 each. Its post-delivery assertion
refinement was restaged and both staged suites rerun before committing. After
the final C011 commit, rebuilt and ran both complete working-tree suites again:
Debug 99/99 and Release 99/99, still with -Werror. Source and tests match HEAD;
current dated documentation and calendar evidence intentionally remain uncommitted.

Actual commands (PowerShell, from C:/Coding Projects/Egalis):

```powershell
$env:PATH = 'C:/msys64/ucrt64/bin;' + $env:PATH
& './build/tooling/cmake/data/bin/cmake.exe' --build build/batch2-debug --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/batch2-debug --output-on-failure
& './build/tooling/cmake/data/bin/cmake.exe' --build build/batch2-release --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/batch2-release --output-on-failure
& './build/milestone8-audit/stage-and-test.ps1' C010 91
& './build/milestone8-audit/commit.ps1' C010
& './build/milestone8-audit/stage-and-test.ps1' C011 99
& './build/milestone8-audit/commit.ps1' C011
```

stage-and-test.ps1 uses alternate indexes and checkout-index source exports,
then fresh CMake/Ninja Debug/Release builds with GCC 14.2.0, -Werror and the pinned
installed GoogleTest prefix. Per-slice diffs, validated tree records, commit
metadata and staged configure/build/test logs are retained in milestone8-audit.
Final post-commit logs are final-Debug-build.txt, final-Debug-tests.txt,
final-Release-build.txt and final-Release-tests.txt in that directory. Staging and
committing never checked out or discarded the user's current planning documents.

Remaining correctness scope: no preaccept, validation, agreement, graph execution,
durability or restart-safety claim follows from these oracle tests. The future
Figure 5 invalidator predicate is phase-sensitive and must not be replaced by this
dependency scan. Retained executed records must not be pruned without a separate
proof/design. No blocker was found for the next dependency-ready milestone 9;
it has not started. No remote or push was configured or performed.

Final integrity audit passed: all original nine commit hashes remain in order;
both new commits have their approved identities, dates, parents, messages and
validated tree hashes. Compared every staged-export blob with its committed tree
(50 files for C010, 51 for C011). All 83 original test names remain in the final
99-case suite. No implementation drift, staged changes, generated artifacts or
remote was found. Current working-tree differences are documentation/evidence
only; all prior planning content is retained with updated status and appended
actual verification. No additional engineering commit was created.

## Milestone 9 implementation and verification — 15 October 2026

Completed C012–C014 in order; stopped before milestone 10. July metadata is an
approved reconstructed calendar, not the date these commands were executed.
The pinned EPaxos* v1 Section 3 / Figure 3 is the implementation reference:
https://arxiv.org/html/2511.02743v1#S3.SS2 . See preaccept.md for the pending
self-request staging refinement and exact duplicate-response replay policy.

| Slice | Local commit | Author = committer | Full Debug / Release, working and staged |
| --- | --- | --- | --- |
| C012 | `1caf07efe8fa053358ef2f37d04a54c216cf27fb` | 2026-07-07T17:10:00-04:00 | 104/104 each |
| C013 | `3f300f0b7c310845846e30d1b24d233d7e96da46` | 2026-07-10T09:50:00-04:00 | 114/114 each |
| C014 | `32e0c855d96af7154872d9e27352d2c7d3219005` | 2026-07-10T12:10:00-04:00 | 122/122 each |

C012 adds checked volatile local allocation, frozen pending initial metadata,
dependency proposals and ordered self-first broadcasts. Trusted preallocated-ID
submission uses the same path, not an alternate protocol. C013 implements the
exact ballot-zero / Initial receive guard, original metadata, current dependency
union and owned PreAcceptOK effects. C014 replays the frozen response for exact
duplicates only, rejects conflicting duplicates and avoids repeat transitions.
No quorum collection, commit, slow agreement, recovery or execution was added.

Twenty-three new tests cover allocation and exhaustion, validation/rejection
atomicity, phase guards, read commutativity, dependency expansion, simultaneous
conflicting coordinators, reordered/duplicated requests, original reply stability
after new conflicts, crash generations, retained-memory resume and seeded replay.
All original 99 test cases remain; one endpoint test was renamed from
EndpointDispatchUsesTheSameCoreAndNeverVotesYet to
EndpointDispatchUsesTheSameCoreAndPreacceptsWithoutCommitting. Old assertions
about unsupported PreAccept and one-record tables were adapted to real traffic,
not removed. Initial C012 failures from these obsolete expectations are retained
in C012-initial-tests.txt. The C014 optimized compiler diagnostic is retained in
C014-initial-Release-failure.txt; direct in-place message construction fixes it
without suppressing warnings. A staged verification trailing blank line was
also corrected before isolated builds. No failed run is counted as a pass.

Actual commands (PowerShell, CMake/Ninja, GCC 14.2.0, pinned installed GoogleTest):

```powershell
$env:PATH = 'C:/msys64/ucrt64/bin;' + $env:PATH
& './build/tooling/cmake/data/bin/cmake.exe' --build build/batch2-debug --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/batch2-debug --output-on-failure
& './build/tooling/cmake/data/bin/cmake.exe' --build build/batch2-release --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/batch2-release --output-on-failure
& './build/milestone9-audit/stage-and-test.ps1' C012 104
& './build/milestone9-audit/commit.ps1' C012
& './build/milestone9-audit/stage-and-test.ps1' C013 114
& './build/milestone9-audit/commit.ps1' C013
& './build/milestone9-audit/stage-and-test.ps1' C014 122
& './build/milestone9-audit/commit.ps1' C014
& './build/milestone9-audit/final-audit.ps1'
```

Each stage-and-test invocation uses a separate index, explicit engineering paths
and a boundary-only verification blob, then exports and freshly configures the
exact staged tree in Debug and Release with -Werror. Review diffs, metadata,
validated trees and build/test logs are under ignored build/milestone9-audit.
Final post-commit suites both pass 122/122 (Debug 2.48s, Release 1.64s); logs are
final-Debug-tests.txt and final-Release-tests.txt. No timing assertion uses these
measured runtimes. Historical trees exclude October planning/event documentation.

Final integrity audit verifies all 11 original commit hashes in order, each new
parent/identity/message/date/tree, and every exported source blob against its
committed tree (55, 59 and 59 files respectively). No implementation drift,
staged files, committed generated artifacts or remote was found. The complete
pre-change working-tree backup remains before-working-tree.tar, SHA256
9A6B3923C3BAFCEAB1E159B7D4D0D1A152197FE11D5A02A8DA7CF2D05044A455.
Existing snapshots, backups and review artifacts were not rewritten. Current
October documentation and calendar evidence remain uncommitted intentionally.

Remaining concerns: reply replay is a documented transport refinement, not a
second vote; future handlers must advance phase/ballot before altering the frozen
response. Milestone 10 must count distinct senders, not packets. Volatile resume
retains memory; durable supporting votes and ID reservations are later work.
No protocol-wide safety/liveness, formal correctness or production readiness is
claimed. Milestone 10 is dependency-ready and explicitly unstarted. No push or
remote configuration occurred.

## Publication checkpoint audit — 15 October 2026

Confirmed C001–C014 on main, HEAD 32e0c855d96af7154872d9e27352d2c7d3219005,
with account identity anishsbala / anishsbala@gmail.com. Authenticated GitHub CLI
reports viewer anishsbala; anishsbala/Egalis returned HTTP 404 before creation.
Inspected all tracked modifications, three untracked calendar evidence files,
59 tracked paths, and 6,418 ignored files. Build/dependency/backups are ignored.
All 14 historical trees (158 unique blobs) and pending documentation passed a
credential-pattern/path audit; daily calendar evidence contains only login,
dates and aggregate contribution counts, with no tokens or private repo names.
Pattern scanning is not a formal guarantee of absence of secrets; publication
paths were also reviewed. No code, tests, CMake or CI changed for publication.

Actual PowerShell commands:

```powershell
$env:PATH = 'C:/msys64/ucrt64/bin;' + $env:PATH
& './build/tooling/cmake/data/bin/cmake.exe' --build build/batch2-debug --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/batch2-debug --output-on-failure
& './build/tooling/cmake/data/bin/cmake.exe' --build build/batch2-release --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/batch2-release --output-on-failure
& './build/publish-audit.ps1'
```

Both builds succeeded; Debug passed 122/122 in 3.06s and Release passed 122/122
in 2.55s. These are test runtimes, not benchmark results. A complete prepublication
working-tree backup was saved as ignored build/prepublish-working-tree.tar;
existing milestone snapshots and review artifacts were preserved. Current
documentation is committed only on checkpoint/milestone-9 with the actual date.
Main remains C014, and milestone 10 has not started. See PAUSE_POINT.md for scope,
limitations and resume instructions. Remote publication is newly authorized by
the user; earlier no-remote/no-push entries describe prior sessions.
