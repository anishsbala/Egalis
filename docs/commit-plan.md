# Commit plan: approved reconstructed 2026 calendar

This replaces the previous calendar proposal. **No Egalis commit is assigned to
2025.** All author and committer timestamps below are proposed reconstructed metadata
for personal visualization, not claims about actual engineering dates. The user
approved this schedule and decomposition, retaining two conditional optimization
commits. Milestones 1–9 (C001–C014) are committed locally; remaining slices require real
implementation and verification, not quota fulfillment. No remote or push is
authorized. Identity remains
`anishsbala <anishsbala@gmail.com>`; branch remains `main`.

Planning target: **112 meaningful commits on 30 dates**, or **110** if profiling
does not justify milestone 35's two optional commits. July receives 45, August 40,
September 16 and October 1–14 receives 11 (9 without the optional index).
All selected dates currently have zero contributions. Sessions contain 2–5 commits,
with 3–6 the general preference; no uniform daily streak is attempted.
Dates are America/New_York, UTC-04:00 throughout this target window. Author and
committer dates are identical for each row. Only C001–C014 have been applied.

Evidence: [authenticated daily snapshot](contribution-calendar-2026.json),
[GraphQL input](contribution-calendar-query.json), and
[calendar analysis / weekly distribution](contribution-calendar-analysis.md).
Fetched 2026-10-15 for viewer `anishsbala`, inclusive 2026-07-01–2026-10-14.
The snapshot contains 106 days, 69 existing contributions, 87 empty days,
9 one-contribution days, and 10 days with at least two contributions. Existing
counts and GitHub color thresholds can change; projections assume eligible commits,
and do not promise a specific shade.

## Local history finalized — 15 October 2026

The approved reconstructed dates have been applied to C001–C006 only. These are
actual local commits, unlike the original recovery tree objects listed below.
October calendar evidence and this dated status update remain uncommitted; they
are deliberately absent from the July-dated trees.

| Milestone | Commit | Author = committer timestamp |
| --- | --- | --- |
| 1 | `e8b839db2e984c2338050538295a8ac4aae2c607` | 2026-07-01T09:35:00-04:00 |
| 2 | `1f5145eb7239bdf4063c582139c09b7bacdafaed` | 2026-07-01T11:10:00-04:00 |
| 3 | `86059f0f6d4cffc58b0e32fc176ddf530dfffe98` | 2026-07-01T14:05:00-04:00 |
| 4 | `8675efee39b2aa7739658d1f211245ac872a023d` | 2026-07-01T16:40:00-04:00 |
| 5 | `205c305089c7a8188c5671f9e50f2f6bcb4e91b3` | 2026-07-02T10:20:00-04:00 |
| 6 | `cdd8ef48ce2dd692ef48aaa35daa730cc6ae7444` | 2026-07-02T11:35:00-04:00 |

Only README, roadmap, boundary commit plan and boundary verification documents
changed from each original reviewed tree. All original implementation, tests,
CMake and CI bytes are preserved. New trees were revalidated independently before
committing. The original trees, backups, alternate indexes and review artifacts
remain unchanged. Detailed current logs and metadata are retained under ignored
`build/approved-history-2026/`; see [verification.md](verification.md).

The remaining 106 proposed slices include two conditional optimizations. Do not
start milestone 7 until the next authorized session. No remote or push is authorized.

## Milestone 7 completed locally — 15 October 2026

| Slice | Commit | Author = committer timestamp | Verification |
| --- | --- | --- | --- |
| C007 | `3f51749ffe94f2c2afae48d8a2b2362188af90de` | 2026-07-02T13:15:00-04:00 | Debug 60/60 |
| C008 | `0d5b1a1a7d968e361d814733c37f7378c6062f81` | 2026-07-02T15:10:00-04:00 | Debug 67/67 |
| C009 | `ccc4c98b11d74385a04cf41e136dfd7d6c19f2a2` | 2026-07-02T17:25:00-04:00 | Final Debug and Release 83/83 each |

C009 incorporates the post-commit optimized-build fix (direct in-place effect
construction), preserving C008 and all six foundation commits. Dispatch and
runtime infrastructure are complete; agreement is deliberately unimplemented.
There are 103 remaining proposed slices, including two conditional optimizations,
not an unconditional quota. Milestone 8 is next; it has not started.
Actual dated verification and planning evidence remain uncommitted and separate
from the reconstructed historical documentation in these local commits.

## Milestone 8 completed locally — 15 October 2026

| Slice | Commit | Author = committer timestamp | Full Debug / Release |
| --- | --- | --- | --- |
| C010 | `b58c9d98aa83811cb88466eb1f556c063cbbbc71` | 2026-07-07T10:05:00-04:00 | 91/91 each |
| C011 | `5be893c06070e69ac4304ee89ab3c07f4dc70db7` | 2026-07-07T13:40:00-04:00 | 99/99 each |

Each exact staged tree passed isolated Debug/Release builds with -Werror before
committing. After C011 both full working-tree suites passed 99/99 again. Original
C001–C009 commits remain unchanged. Known-conflict scans and proposal expansion
are read-only; no agreement or recovery handler was added. See dependency-tracking.md.
There are 101 remaining planned slices including the two conditional optimizations,
not an unconditional quota. Milestone 9 is dependency-ready but unstarted.
Current dated verification and planning evidence remain uncommitted, separate
from historical commit metadata. No remote or push is authorized.

## Existing six reviewed boundaries

Retain milestones 1–6 as six commits. Their code, tests and implementation order
are already independently reviewed and verified. Milestones 1–4 are small cohesive
contracts/components; splitting them further would separate closely related tests
or invent artificial divisions. Milestones 5 and 6 *could* support scheduler vs
transport or fault vs replay splits, but these intermediate states have not been
independently validated. They are unnecessary to reach this plan's target, so the
recommendation is to preserve the six approved source boundaries intact.

The original six **tree objects, not commits**, remain recovery points:

| Milestone / new commit | Original reviewed Git tree | New author = committer date | Original and reconstructed validation |
| --- | --- | --- | --- |
| M1 / C001 | `6078625822ab6d086b0aa595fc567829dbade790` | 2026-07-01T09:35:00-04:00 | 1/1 |
| M2 / C002 | `8bd8c1559797f122a8a4046c0b41eb72cfb64902` | 2026-07-01T11:10:00-04:00 | 6/6 |
| M3 / C003 | `2ff71d9b240b298af152156673a381116046b18b` | 2026-07-01T14:05:00-04:00 | 10/10 |
| M4 / C004 | `df98673135b19ff2e438d35f42c7c3d4dea4321b` | 2026-07-01T16:40:00-04:00 | 19/19 |
| M5 / C005 | `09c4318d59654681f4a5c2121bea8b374f3ffdae` | 2026-07-02T10:20:00-04:00 | 30/30 |
| M6 / C006 | `57e5500757f5504a711d03ea5e5855a58a165940` | 2026-07-02T11:35:00-04:00 | 56/56 |

Original archives remain under `build/milestones/`; reviewed trees, alternate
indexes, exported sources, logs, backup archive and byte-hash manifests remain in
`build/history-preparation/a08bcd44afc749c398271347550bcace/`. Preserve them unchanged.
Their old calendar documents are superseded historical planning artifacts, not
authorized commit instructions. New documentation differs from those old trees;
after approval, refresh documentation at each boundary and validate the resulting
staged trees, while preserving the reviewed implementation hashes. Do not silently
reuse the obsolete calendar or claim new source trees have already been tested.

## Development batches and all 36 milestone counts

[roadmap.md](roadmap.md) remains the acceptance/dependency authority for milestones.
A milestone is complete only when all required slices and its acceptance criteria
are satisfied; early slices are not a complete cluster or recovery implementation.

| Batch | Milestones: proposed commit counts | Total |
| --- | --- | --- |
| 1. Foundation and simulator | M1:1, M2:1, M3:1, M4:1, M5:1, M6:1 | 6 |
| 2. Protocol and agreement | M7:3, M8:2, M9:3, M10:4, M11:4, M12:3 | 19 |
| 3. Graphs and execution | M13:3, M14:3, M15:3 | 9 |
| 4. Recovery and durable storage | M16:4, M17:3, M18:4, M19:5, M20:4, M21:4, M22:4, M23:5, M24:3, M25:3, M26:4, M27:3 | 46 |
| 5. Networking and deployment | M28:4, M29:4, M30:3, M31:3, M32:2 | 16 |
| 6. Fault testing, benchmarks and docs | M33:4, M34:5, M35:2 conditional, M36:5 | 16 / 14 |

M8 and M32 support two substantive slices, not a forced three-commit minimum.
M35 supports at most two justified slices and may be omitted entirely. No extra
format-only, empty-interface, redundant-test or benchmark-number commits are planned.

## Per-date allocation

All original counts in this table are zero, as returned by authenticated GraphQL.
Ranges identify planned commit IDs, not pre-existing commit hashes. Every ID has
its own exact timestamp and acceptance criteria in the detailed ledger.

| Proposed date | Existing count | Proposed commits | IDs | Milestones touched |
| --- | --- | --- | --- | --- |
| 2026-07-01 | 0 | 4 | C001–C004 | M1, M2, M3, M4 |
| 2026-07-02 | 0 | 5 | C005–C009 | M5, M6, M7 |
| 2026-07-07 | 0 | 3 | C010–C012 | M8, M9 |
| 2026-07-10 | 0 | 4 | C013–C016 | M9, M10 |
| 2026-07-13 | 0 | 5 | C017–C021 | M10, M11 |
| 2026-07-16 | 0 | 4 | C022–C025 | M11, M12 |
| 2026-07-21 | 0 | 3 | C026–C028 | M13 |
| 2026-07-24 | 0 | 5 | C029–C033 | M14, M15 |
| 2026-07-27 | 0 | 4 | C034–C037 | M15, M16 |
| 2026-07-29 | 0 | 4 | C038–C041 | M16, M17 |
| 2026-07-30 | 0 | 4 | C042–C045 | M18 |
| 2026-08-03 | 0 | 4 | C046–C049 | M19 |
| 2026-08-05 | 0 | 3 | C050–C052 | M19, M20 |
| 2026-08-09 | 0 | 5 | C053–C057 | M20, M21 |
| 2026-08-11 | 0 | 4 | C058–C061 | M21, M22 |
| 2026-08-15 | 0 | 4 | C062–C065 | M22, M23 |
| 2026-08-17 | 0 | 3 | C066–C068 | M23, M24 |
| 2026-08-21 | 0 | 5 | C069–C073 | M24, M25 |
| 2026-08-24 | 0 | 4 | C074–C077 | M26 |
| 2026-08-27 | 0 | 4 | C078–C081 | M27, M28 |
| 2026-08-29 | 0 | 4 | C082–C085 | M28, M29 |
| 2026-09-09 | 0 | 3 | C086–C088 | M29 |
| 2026-09-14 | 0 | 4 | C089–C092 | M30, M31 |
| 2026-09-19 | 0 | 3 | C093–C095 | M31, M32 |
| 2026-09-24 | 0 | 2 | C096–C097 | M32, M33 |
| 2026-09-28 | 0 | 4 | C098–C101 | M33, M34 |
| 2026-10-02 | 0 | 3 | C102–C104 | M34 |
| 2026-10-07 | 0 | 3 | C105–C107 | M34, M35 |
| 2026-10-12 | 0 | 3 | C108–C110 | M36 |
| 2026-10-14 | 0 | 2 | C111–C112 | M36 |

C106–C107 on October 7 are conditional M35 work; without them that date has one
real benchmark-report commit and late-period allocation is 25, not 27. Keep the
same commit IDs for review if optional rows are skipped; do not create empty
commits or replacement filler.

## Dependency and engineering rules

Every commit follows its preceding mandatory commit in this chronological plan,
plus the explicit parent-milestone prerequisites below. This conservative total
order preserves all original dependencies. C108 depends on C105 when optional
M35 is omitted, or on C107 when it is retained. No features are scheduled before
their required foundations. All milestones before M7 remain untouched in product
scope; M7 is not started during planning.

Each implementation commit must be independently buildable and useful. Tests for
new behavior accompany the implementation; later test commits add distinct fault
cases/counterexamples, not postponed basic coverage. An interface slice must have
a real validated contract used by the next slice, not an empty placeholder. If a
suggested split is not independently useful/buildable when implemented, merge it
and reduce the calendar count. Correctness takes precedence over this count target.

Before a code commit: inspect the relevant pinned specification and current
interfaces; implement; test deterministically; configure/build; run relevant and
existing suites; inspect the staged parent-relative diff; update actual verification
results; then, only after schedule approval, apply the specified author/committer
date and confirm identity, parent and resulting tree. No fabricated results,
benchmarks or formal-correctness claims. The simulator and server must use the same
Replica core. Do not import optimized/thrifty/original EPaxos recovery. Keep
restart refinement and all unresolved protocol policy gates from design.md.

## Detailed commit ledger

Paths below are logical components in the existing repository structure, not a
requirement to manufacture a separate file for each row. All rows inherit the
build/test/diff gate above. Existing verification counts apply only to original
reviewed boundaries; future rows are explicitly unimplemented. No commit, date
or source rewrite is performed by this ledger.

### M1: Build and design harness — 1 commit

Milestone prerequisites: none. Completion anchor: C001.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C001 | Preserve reviewed build and design scaffold | None | CMakeLists.txt; CI; build contract; design/workflow docs | Original 1/1; isolated staged rebuild 1/1; no later source targets | `build: establish C++20 and GoogleTest harness` | 2026-07-01T09:35:00-04:00 | Implemented; revalidated; committed locally |

### M2: Baseline types and ballots — 1 commit

Milestone prerequisites: M1 complete (C001). Completion anchor: C002.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C002 | Preserve checked identities, ballots and quorums | C001; M1 (C001) | protocol/types.hpp/.cpp; types tests | Original and staged 6/6; encoding/ownership/exhaustion and distinct votes | `protocol: define baseline instance and ballot types` | 2026-07-01T11:10:00-04:00 | Implemented; revalidated; committed locally |

### M3: Deterministic commands and conflicts — 1 commit

Milestone prerequisites: M2 complete (C002). Completion anchor: C003.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C003 | Preserve deterministic application semantics | C002; M2 (C002) | kv/command.hpp/.cpp; command tests | Original and staged 10/10; 1,476 commutativity cases; missing/empty/CAS | `kv: define deterministic commands and conflicts` | 2026-07-01T14:05:00-04:00 | Implemented; revalidated; committed locally |

### M4: Baseline transition/message contract — 1 commit

Milestone prerequisites: M2 complete (C002); M3 complete (C003). Completion anchor: C004.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C004 | Preserve baseline message and transition contract | C003; M2 (C002), M3 (C003) | protocol/messages.hpp/.cpp; baseline-transitions.md; tests | Original and staged 19/19; baseline-only guards and all alternatives | `protocol: extract baseline transitions and messages` | 2026-07-01T16:40:00-04:00 | Implemented; revalidated; committed locally |

### M5: Scheduler and typed transport — 1 commit

Milestone prerequisites: M4 complete (C004). Completion anchor: C005.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C005 | Preserve reviewed scheduler and typed transport | C004; M4 (C004) | sim/scheduler and network; transport.hpp; simulation tests | Original and staged 30/30; ties, ownership, self-send and overflow | `sim: add deterministic network scheduler` | 2026-07-02T10:20:00-04:00 | Implemented; revalidated; committed locally |

### M6: Fault injection and replay — 1 commit

Milestone prerequisites: M5 complete (C005). Completion anchor: C006.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C006 | Preserve final fault and replay implementation | C005; M5 (C005) | sim faults/random/trace; review regressions; ADR; CI | Original 56/56 Debug/Release; staged 56/56; code/fixtures retained | `sim: add fault schedules and trace replay` | 2026-07-02T11:35:00-04:00 | Implemented; revalidated; committed locally |

### M7: Replica event dispatch — 3 commits

Milestone prerequisites: M4 complete (C004); M6 complete (C006). Completion anchor: C009.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C007 | Define the replica's explicit input/effect contract | C006; M4 (C004), M6 (C006) | protocol events/effects alongside existing messages; contract tests | Typed submit/message/timer/completion inputs; no socket or clock dependency | `protocol: define replica events and effects` | 2026-07-02T13:15:00-04:00 | Implemented; verified; committed locally |
| C008 | Implement serialized dispatch with one instance table | C007; M4 (C004), M6 (C006) | Replica event entry point and authoritative instance state; unit tests | Deterministic event routing and state ownership; unsupported phases are not advertised as implemented | `protocol: add serialized replica event dispatch` | 2026-07-02T15:10:00-04:00 | Implemented; verified; committed locally |
| C009 | Connect simulator endpoints to the shared replica core | C008; M4 (C004), M6 (C006) | sim endpoint and timer effect adapter; integration tests | Same Replica class invoked through Endpoint; synchronous self-effects and stale-event guards | `sim: run shared replica core through typed endpoints` | 2026-07-02T17:25:00-04:00 | Implemented; verified; committed locally |

### M8: Known-conflict dependencies — 2 commits

Milestone prerequisites: M3 complete (C003); M7 complete (C009). Completion anchor: C011.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C010 | Implement complete known-conflict dependency scans | C009; M3 (C003), M7 (C009) | protocol dependency helper over InstanceState; oracle tests | Other-instance scan; no omitted known/executed commands; canonical output | `protocol: compute dependencies from known conflicts` | 2026-07-07T10:05:00-04:00 | Implemented; verified; committed locally |
| C011 | Preserve visibility through payload and initial-metadata changes | C010; M3 (C003), M7 (C009) | dependency helper and cross-phase fixtures; invariant comments | Nop/bottom rules and original metadata remain visible; no second state table or pruning | `protocol: preserve dependency visibility across candidate changes` | 2026-07-07T13:40:00-04:00 | Implemented; verified; committed locally |

### M9: Initial preaccept — 3 commits

Milestone prerequisites: M8 complete (C011). Completion anchor: C014.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C012 | Allocate a unique initial instance on submission | C011; M8 (C011) | submit path and in-memory ID allocation; tests | No ID reuse/overflow; initial coordinator proposal includes known conflicts | `protocol: submit commands with checked instance allocation` | 2026-07-07T17:10:00-04:00 | Implemented, verified and committed locally |
| C013 | Implement initial PreAccept handling | C012; M8 (C011) | PreAccept receive transition and PreAcceptOK effects; tests | Exact bal/phase guard; initial metadata retained; deterministic dependency union | `protocol: implement baseline preaccept transition` | 2026-07-10T09:50:00-04:00 | Implemented, verified and committed locally |
| C014 | Handle identical duplicate preaccept requests safely | C013; M8 (C011) | reply caching/duplicate guards; replay and reorder tests | No repeated transition or conflicting duplicate acceptance; review against specification | `protocol: replay preaccept responses without voting twice` | 2026-07-10T12:10:00-04:00 | Implemented, verified and committed locally |

### M10: Fast agreement — 4 commits

Milestone prerequisites: M9 complete (C014). Completion anchor: C018.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C015 | Collect distinct fast-path observations | C014; M9 (C014) | initial coordinator vote collector; quorum tests | Sender/instance/ballot matching and exact initial-dependency equality; freeze proposal inputs | `protocol: collect distinct preaccept observations` | 2026-07-10T14:35:00-04:00 | Planned; not implemented |
| C016 | Commit four matching initial votes | C015; M9 (C014) | fast Commit send/receive guards in shared core; scenario tests | Four distinct voters; exact ballot receive guard; no arbitrary equal-dependency fast path | `protocol: commit baseline fast-path decisions` | 2026-07-10T18:05:00-04:00 | Planned; not implemented |
| C017 | Give fast agreement a deterministic grace policy | C016; M9 (C014) | logical timer policy and slow-fallback effect; ADR/tests | Do not fall back at third equal reply; bounded fallback with three survivors; one frozen proposal | `protocol: schedule deterministic fast-path fallback` | 2026-07-13T09:25:00-04:00 | Planned; not implemented |
| C018 | Exercise fast-path races and forbidden decisions | C017; M9 (C014) | fast agreement fault fixtures and guard tests | Reorders, duplicates, conflicts and coordinator crash check safety; no recovery liveness claim | `test: cover fast-path quorum and coordinator races` | 2026-07-13T10:50:00-04:00 | Planned; not implemented |

### M11: Slow agreement — 4 commits

Milestone prerequisites: M10 complete (C018). Completion anchor: C022.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C019 | Freeze the slow-path dependency union | C018; M10 (C018) | slow proposal construction and coordinator state; tests | Union selected distinct replies once; late replies cannot change the proposal | `protocol: freeze slow-path dependency proposals` | 2026-07-13T13:30:00-04:00 | Planned; not implemented |
| C020 | Accept proposals under precise ballot guards | C019; M10 (C018) | Accept/AcceptOK receive transitions; tests | Separate bal/abal; exact equal-ballot committed exclusion; immutable chosen values | `protocol: implement ballot-guarded accept transitions` | 2026-07-13T15:45:00-04:00 | Planned; not implemented |
| C021 | Commit after a matching slow quorum | C020; M10 (C018) | AcceptOK collection and common Commit path; tests | Three distinct matching votes; reject stale acknowledgments; reuse commit handling | `protocol: commit slow-path quorum decisions` | 2026-07-13T18:15:00-04:00 | Planned; not implemented |
| C022 | Test interrupted slow agreement and stale messages | C021; M10 (C018) | fault fixtures through each slow-phase cut; integration tests | Two failed peers, duplicates and superseded ballots preserve decisions; no invented recovery | `test: cover interrupted slow agreement and stale commits` | 2026-07-16T10:10:00-04:00 | Planned; not implemented |

### M12: Agreement/visibility observation — 3 commits

Milestone prerequisites: M11 complete (C022). Completion anchor: C025.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C023 | Observe agreement after every simulator event | C022; M11 (C022) | cross-replica agreement observer; negative fixtures | Deliberately inconsistent committed payload/dependencies are detected at first violation | `sim: check committed agreement after each event` | 2026-07-16T12:45:00-04:00 | Planned; not implemented |
| C024 | Observe dependency visibility for conflicting commits | C023; M11 (C022) | visibility observer and conflict-aware graph fixtures | Detect missing conflict edge; ignore legitimate independent command ordering | `sim: check visibility of conflicting decisions` | 2026-07-16T15:05:00-04:00 | Planned; not implemented |
| C025 | Generate concurrent agreement histories with replay artifacts | C024; M11 (C022) | seeded submission driver and regression artifacts | Assert fast/slow/conflict coverage; reproduce first safety failure without clocks | `test: explore concurrent agreement histories` | 2026-07-16T17:50:00-04:00 | Planned; not implemented |

### M13: Committed graph closure — 3 commits

Milestone prerequisites: M11 complete (C022). Completion anchor: C028.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C026 | Build graph views from committed instance records | C025; M11 (C022) | execution graph view and vertex lookup; tests | No duplicated authoritative consensus state; edges point from dependencies to dependents | `execution: construct graph views from committed instances` | 2026-07-21T11:20:00-04:00 | Planned; not implemented |
| C027 | Compute the closed committed dependency subgraph | C026; M11 (C022) | closure algorithm and small graph fixtures | Missing/uncommitted transitive vertices block the whole dependent closure; Nop retained | `execution: compute closed committed dependency graphs` | 2026-07-21T14:15:00-04:00 | Planned; not implemented |
| C028 | Report unresolved dependencies on commit notifications | C027; M11 (C022) | closure/core integration and missing-ID reports; tests | New commits unblock only eligible vertices; report recovery needs without implementing recovery early | `execution: update readiness on dependency commits` | 2026-07-21T17:35:00-04:00 | Planned; not implemented |

### M14: SCC execution order — 3 commits

Milestone prerequisites: M13 complete (C028). Completion anchor: C031.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C029 | Find strongly connected components deterministically | C028; M13 (C028) | SCC algorithm and cycle fixtures | Self/cyclic graph cases, canonical traversal, correct component partition | `execution: find strongly connected dependency components` | 2026-07-24T09:40:00-04:00 | Planned; not implemented |
| C030 | Order components dependency first and break internal ties | C029; M13 (C028) | condensation ordering and numerical ID ordering; tests | Dependencies precede dependents; fixed ID order inside SCC; no original EPaxos sequence numbers | `execution: order dependency components deterministically` | 2026-07-24T11:25:00-04:00 | Planned; not implemented |
| C031 | Validate execution orders under graph permutations | C030; M13 (C028) | permutation and multi-replica graph regressions | Shuffled insertion/layout preserves conflicting order; independent component alternatives remain valid | `test: verify execution ordering across graph permutations` | 2026-07-24T13:55:00-04:00 | Planned; not implemented |

### M15: KV execution and results — 3 commits

Milestone prerequisites: M3 complete (C003); M14 complete (C031). Completion anchor: C034.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C032 | Apply ready commands once through the KV state machine | C031; M3 (C003), M14 (C031) | executor to kv::StateMachine integration; tests | Only closed committed work applies; repeated scheduling cannot reapply an instance | `kv: apply committed commands exactly once per instance` | 2026-07-24T16:10:00-04:00 | Planned; not implemented |
| C033 | Release client results after local execution | C032; M3 (C003), M14 (C031) | pending-result effects and core/executor integration; tests | GET/CAS responses match applied state; commit alone never acknowledges completion | `protocol: return client results after execution` | 2026-07-24T18:20:00-04:00 | Planned; not implemented |
| C034 | Settle Nop instances without application effects | C033; M3 (C003), M14 (C031) | Nop execution bookkeeping and converged-history tests | Nop vertices satisfy readiness; no KV mutation/result masquerading as the original command | `execution: settle nop instances without application effects` | 2026-07-27T10:30:00-04:00 | Planned; not implemented |

### M16: Recovery ballots and snapshots — 4 commits

Milestone prerequisites: M11 complete (C022); M12 complete (C025). Completion anchor: C038.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C035 | Start recovery with a checked owned ballot | C034; M11 (C022), M12 (C025) | recover input transition; ballot tests and source mapping | Strictly higher owned ballot; no reset/reuse; recovery still incomplete | `protocol: initiate recovery with owned ballots` | 2026-07-27T12:20:00-04:00 | Planned; not implemented |
| C036 | Join a recovery ballot and report prior accepted state | C035; M11 (C022), M12 (C025) | Recover/RecoverOK handling; snapshot tests | Persistable promise state differs from abal; replayed replies do not manufacture votes | `protocol: report state when joining recovery ballots` | 2026-07-27T14:50:00-04:00 | Planned; not implemented |
| C037 | Freeze the recovery quorum and classify highest accepted records | C036; M11 (C022), M12 (C025) | recovery collector and snapshot representation; tests | Distinct matching Q; maximum abal subset; late replies cannot replace frozen observations | `protocol: freeze recovery quorum snapshots` | 2026-07-27T17:15:00-04:00 | Planned; not implemented |
| C038 | Invalidate superseded recovery continuations | C037; M11 (C022), M12 (C025) | attempt tokens/ballot guards and competing-coordinator fixtures | Old timers/responses cannot revive an attempt; retain accepted value evidence | `protocol: guard competing recovery attempts` | 2026-07-29T09:55:00-04:00 | Planned; not implemented |

### M17: Recover accepted decisions — 3 commits

Milestone prerequisites: M16 complete (C038). Completion anchor: C041.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C039 | Rebroadcast the highest committed recovery decision | C038; M16 (C038) | committed recovery branch and tests | Highest-abal committed branch wins; same payload and dependency set under new ballot | `protocol: recover previously committed decisions` | 2026-07-29T11:45:00-04:00 | Planned; not implemented |
| C040 | Resume the highest accepted proposal | C039; M16 (C038) | accepted recovery branch using common Accept path; tests | Only highest-abal evidence; no recomputed dependency union or payload substitution | `protocol: resume highest accepted recovery proposals` | 2026-07-29T14:25:00-04:00 | Planned; not implemented |
| C041 | Test crashes around recovered slow decisions | C040; M16 (C038) | recovery fixtures before/after quorum and broadcast | Chosen values survive repeat recovery and coordinator crashes; branch precedence asserted | `test: preserve slow decisions across repeated recovery` | 2026-07-29T16:55:00-04:00 | Planned; not implemented |

### M18: Validate possible fast decisions — 4 commits

Milestone prerequisites: M17 complete (C041). Completion anchor: C045.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C042 | Select possible fast decisions from baseline evidence | C041; M17 (C041) | baseline candidate predicate and source-mapped tests | Exact Section 3 R condition; no highlighted optimized/thrifty branches | `protocol: select baseline recovery candidates` | 2026-07-30T10:15:00-04:00 | Planned; not implemented |
| C043 | Implement validation without creating preaccept votes | C042; M17 (C041) | Validate receive transition and invalidator predicate; tests | Update cmd/initCmd/initDep only as specified; preserve phase/abal distinctions | `protocol: validate possible fast-path decisions` | 2026-07-30T12:30:00-04:00 | Planned; not implemented |
| C044 | Aggregate validation over the original quorum | C043; M17 (C041) | ValidateOK collection and immediate recovery outcomes; tests | Every original Q member required; empty I recovers candidate; committed invalidation proposes Nop | `protocol: decide immediate validation outcomes` | 2026-07-30T15:20:00-04:00 | Planned; not implemented |
| C045 | Reproduce the paper's resurrected-command counterexample | C044; M17 (C041) | Example 6 adaptation and candidate-preservation fixtures | Reject unsafe resurrection; assert validation and invalidating command are actually reached | `test: reject unsafe command resurrection during recovery` | 2026-07-30T18:10:00-04:00 | Planned; not implemented |

### M19: Resolve invalidators and waiting — 5 commits

Milestone prerequisites: M18 complete (C045). Completion anchor: C050.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C046 | Watch potentially invalidating commands | C045; M18 (C045) | recovery watches keyed to attempt and command; tests | Keep observed I separate from current resolution state; missing progress does not count as approval | `protocol: track potentially invalidating commands` | 2026-08-03T09:30:00-04:00 | Planned; not implemented |
| C047 | Broadcast baseline waiting evidence | C046; M18 (C045) | Waiting(id) effects and attempt state; tests | No R_max/threshold fields; only active waiting attempts emit evidence | `protocol: announce blocked baseline recoveries` | 2026-08-03T11:40:00-04:00 | Planned; not implemented |
| C048 | Resolve watched commands after commitment | C047; M18 (C045) | waiting continuation cases for candidate/Nop; tests | All safe resolutions or any committed invalidator trigger exact documented branch | `protocol: resolve recovery waits on committed commands` | 2026-08-03T14:10:00-04:00 | Planned; not implemented |
| C049 | Break baseline recovery waiting cycles | C048; M18 (C045) | cross-Waiting escape branch and mutual-blocking fixtures | Apply baseline guard only; chosen fast decisions cannot be aborted by unrelated waiting | `protocol: break baseline recovery waiting cycles` | 2026-08-03T16:45:00-04:00 | Planned; not implemented |
| C050 | Exercise late validation and waiting messages | C049; M18 (C045) | stale-attempt, coordinator crash and mutual-wait regressions | No stale continuation changes a chosen value; unresolved outcomes remain explicit | `test: cover stale validation and recovery waiting races` | 2026-08-05T10:45:00-04:00 | Planned; not implemented |

### M20: Recovery initiation and resubmission — 4 commits

Milestone prerequisites: M19 complete (C050); M15 complete (C034). Completion anchor: C054.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C051 | Route recovery requests through detector nominations | C050; M19 (C050), M15 (C034) | TryRecover dispatch and injectable detector input; tests | Only nominated active coordinator starts an attempt; eventual convergence is an explicit assumption | `protocol: route recovery through coordinator nominations` | 2026-08-05T13:25:00-04:00 | Planned; not implemented |
| C052 | Schedule bounded recovery retries on logical timers | C051; M19 (C050), M15 (C034) | increasing-delay retry policy and generation/ballot cancellation; tests | No sleeps; stale retry events ignored; no promise of progress under permanent partition | `protocol: schedule recovery retries with increasing delays` | 2026-08-05T17:05:00-04:00 | Planned; not implemented |
| C053 | Recover unresolved dependency instances | C052; M19 (C050), M15 (C034) | closure reports to recovery policy integration; scenarios | Missing vertices trigger real recovery; recovered decisions unblock execution | `protocol: recover missing execution dependencies` | 2026-08-09T09:20:00-04:00 | Planned; not implemented |
| C054 | Resubmit owner commands replaced by Nop | C053; M19 (C050), M15 (C034) | owner resubmission and quiescent-liveness scenarios | New instance for original operation; no cross-replica exactly-once claim before M26; f=2 completion under stated assumptions | `protocol: resubmit commands replaced by nop` | 2026-08-09T11:05:00-04:00 | Planned; not implemented |

### M21: Bounded recovery exploration — 4 commits

Milestone prerequisites: M20 complete (C054). Completion anchor: C058.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C055 | Enumerate bounded recovery schedules | C054; M20 (C054) | bounded interleaving driver and oracle fixtures | Bounds reported; enumerate small crash/delivery cases without treating exhaustion as proof | `test: enumerate bounded recovery interleavings` | 2026-08-09T13:35:00-04:00 | Planned; not implemented |
| C056 | Reduce only demonstrably equivalent schedules | C055; M20 (C054) | explorer equivalence/reduction logic and brute-force comparisons | Unknown dependence prevents pruning; reduced search matches small full enumeration | `test: reduce equivalent recovery schedules safely` | 2026-08-09T15:50:00-04:00 | Planned; not implemented |
| C057 | Run seeded recovery campaigns with branch coverage | C056; M20 (C054) | fault suite and first-violation capture | Finite submission, fair delivery and detector convergence for progress checks; branch coverage asserted | `test: explore seeded recovery fault campaigns` | 2026-08-09T18:25:00-04:00 | Planned; not implemented |
| C058 | Minimize replayable recovery counterexamples | C057; M20 (C054) | trace reduction tool and saved fixtures | Reduction retains the same violation and reproducibility; rejected reductions cannot yield false success | `test: minimize reproducible recovery counterexamples` | 2026-08-11T10:25:00-04:00 | Planned; not implemented |

### M22: Checksummed WAL — 4 commits

Milestone prerequisites: M21 complete (C058). Completion anchor: C062.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C059 | Encode versioned checksummed protocol records | C058; M21 (C058) | storage WAL codec and record tests | Stable field coverage, bounded lengths and checksum detection; no opaque raw C++ memory layout | `storage: encode versioned checksummed wal records` | 2026-08-11T12:15:00-04:00 | Planned; not implemented |
| C060 | Append and flush complete log records | C059; M21 (C058) | WAL writer and I/O adapter; temporary-file tests | Flush completion is explicit; record ordering and append errors tested | `storage: append and flush protocol log records` | 2026-08-11T14:40:00-04:00 | Planned; not implemented |
| C061 | Replay valid prefixes and handle torn tails | C060; M21 (C058) | WAL reader/replay validation; truncation fixtures | Recover intact prefix; interior corruption fails closed; repair limited to own damaged tail | `storage: replay valid wal prefixes after interruption` | 2026-08-11T17:30:00-04:00 | Planned; not implemented |
| C062 | Exercise short writes and storage failures | C061; M21 (C058) | I/O failure injector and real file regressions | Short writes/flush failures never imply durability; errors stop participation | `test: fail closed on wal write and flush errors` | 2026-08-15T09:45:00-04:00 | Planned; not implemented |

### M23: Durable protocol effects — 5 commits

Milestone prerequisites: M22 complete (C062). Completion anchor: C067.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C063 | Gate promises and effects on durable completion | C062; M22 (C062) | core storage effects/continuations plus recovery promises; tests | No externally visible promise before its batch flush; stale completion cannot resume superseded ballot | `storage: gate recovery promises on durable completion` | 2026-08-15T11:55:00-04:00 | Planned; not implemented |
| C064 | Persist preaccept and initial command metadata | C063; M22 (C062) | durable preaccept records and reply gating; tests | cmd/dep/initCmd/initDep/phase sufficient to replay the original vote before PreAcceptOK | `protocol: persist preaccept state before replies` | 2026-08-15T14:20:00-04:00 | Planned; not implemented |
| C065 | Persist accepted and committed protocol records | C064; M22 (C062) | Accept/Commit durability integration and tests | bal/abal/value/phase recorded together; visible dependent effects follow durability | `protocol: persist accepted and committed decisions` | 2026-08-15T16:50:00-04:00 | Planned; not implemented |
| C066 | Persist validation metadata before observation replies | C065; M22 (C062) | Validate persistence and ValidateOK gating; tests | No hidden validation state loss after emitted reply; do not accidentally persist a preaccept vote | `protocol: persist validation state before replies` | 2026-08-17T10:35:00-04:00 | Planned; not implemented |
| C067 | Test every protocol durability cut | C066; M22 (C062) | crash-cut table and simulated storage/protocol runner | Cuts around append/flush/send across all vote paths; chosen values and visible metadata preserved | `test: cover crashes at protocol durability boundaries` | 2026-08-17T13:50:00-04:00 | Planned; not implemented |

### M24: Identity and protocol replay — 3 commits

Milestone prerequisites: M23 complete (C067). Completion anchor: C070.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C068 | Validate durable replica identity and configuration | C067; M23 (C067) | storage identity header/open checks; tests | Wrong membership/identity rejected; lost state cannot silently reopen as an existing voter | `storage: validate durable replica identity` | 2026-08-17T17:20:00-04:00 | Planned; not implemented |
| C069 | Reserve instance identifier ranges durably | C068; M23 (C067) | ID allocator/WAL reservations; exhaustion tests | Crash before/after reservation cannot reuse an exposed instance ID | `storage: reserve instance identifiers before exposure` | 2026-08-21T09:15:00-04:00 | Planned; not implemented |
| C070 | Reconstruct protocol state from durable records | C069; M23 (C067) | replay-to-instance-table integration and restart refinement ADR | Replay equivalence; rebuild transient attempts through recovery; review refinement before resumed participation | `storage: rebuild protocol state from durable records` | 2026-08-21T11:15:00-04:00 | Planned; not implemented |

### M25: Atomic application persistence — 3 commits

Milestone prerequisites: M24 complete (C070); M15 complete (C034). Completion anchor: C073.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C071 | Atomically record application state and execution results | C070; M24 (C070), M15 (C034) | versioned apply record with KV change/instance/result; tests | One durable batch describes mutation, execution marker and returned result | `storage: atomically record application execution results` | 2026-08-21T13:45:00-04:00 | Planned; not implemented |
| C072 | Replay application state without duplicate effects | C071; M24 (C070), M15 (C034) | application replay and execution bookkeeping; tests | Replaying/restarting repeatedly cannot reapply an already durable operation | `storage: replay applied state without duplicate effects` | 2026-08-21T15:55:00-04:00 | Planned; not implemented |
| C073 | Gate client replies on durable application completion | C072; M24 (C070), M15 (C034) | result effect runner and before/after-flush/reply crash fixtures | Acknowledged results survive restart; timeout retains outcome-unknown semantics | `protocol: reply only after durable application completion` | 2026-08-21T18:00:00-04:00 | Planned; not implemented |

### M26: Logical request deduplication — 4 commits

Milestone prerequisites: M25 complete (C073); M20 complete (C054). Completion anchor: C077.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C074 | Define logical request identity and request-aware conflicts | C073; M25 (C073), M20 (C054) | command envelope/request IDs, versioned fingerprints and tests | Same identity conflicts even across keys; payload mismatch rejected; existing operation semantics retained | `client: define logical request identities and conflicts` | 2026-08-24T10:00:00-04:00 | Planned; not implemented |
| C075 | Deduplicate requests in replicated application state | C074; M25 (C073), M20 (C054) | request/result table within atomic apply; tests | Exactly one logical mutation for repeated instances; deterministic cached response | `kv: deduplicate logical requests during replicated apply` | 2026-08-24T12:35:00-04:00 | Planned; not implemented |
| C076 | Retain request identity through owner resubmission | C075; M25 (C073), M20 (C054) | Nop resubmission and durable request metadata; tests | New consensus instance retains original request; no duplicate client effect | `protocol: preserve request identity across resubmission` | 2026-08-24T15:15:00-04:00 | Planned; not implemented |
| C077 | Test uncertain outcomes and cross-replica retries | C076; M25 (C073), M20 (C054) | retry adapter/internal client driver and failure histories | Lost responses, concurrent duplicate instances and mismatch attempts preserve results | `test: verify retries across replicas and restarts` | 2026-08-24T17:45:00-04:00 | Planned; not implemented |

### M27: Durable crash/rejoin validation — 3 commits

Milestone prerequisites: M26 complete (C077). Completion anchor: C080.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C078 | Restart endpoints with reconstructed durable cores | C077; M26 (C077) | durable simulation runner and endpoint replacement boundary; tests | Same core as steady state; safe ownership replacement and generation invalidation | `sim: reconstruct durable replicas after crashes` | 2026-08-27T09:40:00-04:00 | Planned; not implemented |
| C079 | Catch up lagging replicas from retained protocol history | C078; M26 (C077) | decision dissemination/recovery integration; partition-heal scenarios | No unsafe pruning or snapshot shortcut; lagging graph and application converge | `protocol: catch up lagging replicas after rejoin` | 2026-08-27T11:50:00-04:00 | Planned; not implemented |
| C080 | Exercise repeated restarts and obsolete messages | C079; M26 (C077) | durable fault scenarios and documented refinement limits | Repeated crash cuts, stale packets/timers and at-most-once logical apply; do not claim proof | `test: verify repeated durable crash and rejoin behavior` | 2026-08-27T14:30:00-04:00 | Planned; not implemented |

### M28: Protobuf codecs — 4 commits

Milestone prerequisites: M27 complete (C080). Completion anchor: C084.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C081 | Define the versioned baseline wire schema | C080; M27 (C080) | proto messages and CMake Protobuf integration; schema checks | All current baseline variants/IDs represented; no TCP dependency in core | `transport: define versioned protobuf protocol messages` | 2026-08-27T17:00:00-04:00 | Planned; not implemented |
| C082 | Convert typed messages to and from wire records | C081; M27 (C080) | Protobuf adapters and field validation; unit tests | No variant mixing; preserve ownership/payload/dependency fields and rejection behavior | `transport: convert typed protocol messages to protobuf` | 2026-08-29T10:20:00-04:00 | Planned; not implemented |
| C083 | Test codec round trips and version compatibility | C082; M27 (C080) | golden wire fixtures and schema-version policy | Binary keys, sentinels and all message alternatives round-trip; unsupported version rejected | `test: verify protobuf round trips and compatibility` | 2026-08-29T12:40:00-04:00 | Planned; not implemented |
| C084 | Reject malformed and oversized peer input | C083; M27 (C080) | decoder size/identity checks and malformed fixtures | Bound allocation and framing claims; reject invalid membership/ballots before core dispatch | `transport: reject malformed and oversized protocol messages` | 2026-08-29T15:25:00-04:00 | Planned; not implemented |

### M29: Peer transport — 4 commits

Milestone prerequisites: M28 complete (C084). Completion anchor: C088.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C085 | Choose transport library and implement incremental framing | C084; M28 (C084) | transport ADR plus frame parser/encoder and tests | Real framing functionality; partial/truncated frames tested without sockets or core changes | `transport: add incremental peer message framing` | 2026-08-29T18:10:00-04:00 | Planned; not implemented |
| C086 | Implement peer connections feeding serialized core input | C085; M28 (C084) | socket adapter/connection lifecycle and loopback tests | I/O completions enqueue typed events; no protocol mutation in I/O workers | `transport: connect peers through the shared event boundary` | 2026-09-09T10:10:00-04:00 | Planned; not implemented |
| C087 | Reconnect and retry without changing protocol semantics | C086; M28 (C084) | connection retry/outbox policy and loss/reconnect tests | Eventual delivery once links stabilize; duplicates remain valid inputs; no invented consensus messages | `transport: retry peer messages after reconnection` | 2026-09-09T13:30:00-04:00 | Planned; not implemented |
| C088 | Bound queues and handle slow or failed peers | C087; M28 (C084) | queue limits/backpressure and loopback failure integration | Slow peer cannot cause unbounded buffering; readiness/events replace test sleeps | `transport: bound peer queues and handle backpressure` | 2026-09-09T17:10:00-04:00 | Planned; not implemented |

### M30: Durable server — 3 commits

Milestone prerequisites: M29 complete (C088). Completion anchor: C091.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C089 | Run a configured durable replica executable | C088; M29 (C088) | server app/configuration/startup and local smoke tests | Static five-member identity; use the same durable core and typed transport | `server: start durable replicas from static configuration` | 2026-09-14T09:35:00-04:00 | Planned; not implemented |
| C090 | Implement the concrete coordinator detector policy | C089; M29 (C088) | heartbeat/suspicion adapter and convergence scenarios | Eventually stable per-instance recovery nominations under assumptions; no global steady-state leader | `server: nominate recovery coordinators from peer health` | 2026-09-14T11:55:00-04:00 | Planned; not implemented |
| C091 | Test multiprocess failures and orderly shutdown | C090; M29 (C088) | server lifecycle/effect draining and process tests | Ready signals, stop/restart and in-flight durable decisions; bounded event-based deadlines | `test: verify server failures and orderly shutdown` | 2026-09-14T14:45:00-04:00 | Planned; not implemented |

### M31: Client API — 3 commits

Milestone prerequisites: M30 complete (C091); M26 complete (C077). Completion anchor: C094.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C092 | Expose the typed KV request/result API and CLI | C091; M30 (C091), M26 (C077) | client schema/adapters/CLI and validation tests | GET/PUT/CAS semantics, request identity and bounded errors agree with core model | `client: expose key-value requests and results` | 2026-09-14T17:25:00-04:00 | Planned; not implemented |
| C093 | Preserve outcome-unknown retries over client connections | C092; M30 (C091), M26 (C077) | client timeouts/reconnect and request reuse; tests | Timeout is not abort; same logical request retains identity and cached result | `client: preserve request identity across connection retries` | 2026-09-19T10:40:00-04:00 | Planned; not implemented |
| C094 | Capture end-to-end KV histories over reconnects | C093; M30 (C091), M26 (C077) | multi-replica API integration/history capture | CAS observed values, reads through consensus, response loss and duplicate request cases | `test: capture key-value histories across client reconnects` | 2026-09-19T14:05:00-04:00 | Planned; not implemented |

### M32: Compose deployment — 2 commits

Milestone prerequisites: M31 complete (C094). Completion anchor: C096.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C095 | Package the five-node local cluster | C094; M31 (C094) | pinned Docker image and Compose topology/volumes | Build actual server; separate durable volume per identity; no remote deployment or publishing | `deploy: package a five-replica compose cluster` | 2026-09-19T17:40:00-04:00 | Planned; not implemented |
| C096 | Automate cluster readiness and restart smoke checks | C095; M31 (C094) | Compose smoke orchestration and operation instructions | Ready-driven start, minority stop/restart, retained state and successful KV calls | `test: verify compose cluster readiness and restart` | 2026-09-24T11:15:00-04:00 | Planned; not implemented |

### M33: History checker — 4 commits

Milestone prerequisites: M31 complete (C094). Completion anchor: C100.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C097 | Define history records and validate imported traces | C096; M31 (C094) | Python 3.12+ history format/parser and fixtures | Client-clock invocation/response ordering, stable request identity; malformed histories fail explicitly | `history: validate client operation histories` | 2026-09-24T16:20:00-04:00 | Planned; not implemented |
| C098 | Search bounded legal sequential histories | C097; M31 (C094) | linearizability search and tiny known-outcome cases | Honor completed real-time precedence; pending calls handled; exhaustion is inconclusive, not pass | `history: check bounded operation linearizability` | 2026-09-28T09:50:00-04:00 | Planned; not implemented |
| C099 | Check the exact KV result semantics | C098; M31 (C094) | sequential model adapter and valid/invalid GET/PUT/CAS fixtures | Missing vs empty and both CAS outcomes checked; known illegal histories rejected | `test: validate the checker against key-value counterexamples` | 2026-09-28T12:05:00-04:00 | Planned; not implemented |
| C100 | Check retry histories and sound key partitioning | C099; M31 (C094) | logical request collapsing/partition policy and integration | No unjustified independence across request IDs; real/simulated histories supported within reported bounds | `history: check retried requests across replica histories` | 2026-09-28T14:35:00-04:00 | Planned; not implemented |

### M34: Reproducible benchmarks — 5 commits

Milestone prerequisites: M32 complete (C096); M33 complete (C100). Completion anchor: C105.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C101 | Generate reproducible offered-load workloads | C100; M32 (C096), M33 (C100) | Python workload generator and deterministic fixtures | Seeded uniform/hot-key access, value sizes and operation mixes; no benchmark numbers invented | `bench: generate reproducible key-value workloads` | 2026-09-28T17:15:00-04:00 | Planned; not implemented |
| C102 | Orchestrate measured runs with environment metadata | C101; M32 (C096), M33 (C100) | local/container run driver and metadata capture | Warmup, resource limits, topology, seed and source revision recorded; smoke measurements labeled | `bench: orchestrate repeatable distributed runs` | 2026-10-02T10:30:00-04:00 | Planned; not implemented |
| C103 | Record raw completion and latency measurements | C102; M32 (C096), M33 (C100) | client/protocol counters and raw artifact writer; tests | Offered vs completed logical throughput, errors/retries and commit/execute/client latencies separated | `bench: record raw throughput and latency samples` | 2026-10-02T13:45:00-04:00 | Planned; not implemented |
| C104 | Analyze measured traces and fault behavior | C103; M32 (C096), M33 (C100) | Python trace/report tooling and labeled synthetic analysis fixtures | Percentiles, fast/slow/recovery counts and stalls trace to raw samples; fixtures not represented as measurements | `bench: analyze protocol latency and recovery traces` | 2026-10-02T17:30:00-04:00 | Planned; not implemented |
| C105 | Publish a local reproducible measurement report | C104; M32 (C096), M33 (C100) | measured artifacts, checked histories and benchmark documentation | Run actual workload; preserve raw data and checker limits; local report only, no external publication | `bench: document measured behavior with reproducible artifacts` | 2026-10-07T09:55:00-04:00 | Planned; not implemented |

### M35: Conditional conflict index — 2 conditional commits

Milestone prerequisites: M34 complete (C105). Completion anchor: C107.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C106 | Implement a derived conflict index only if profiling warrants it | C105; M34 (C105) | conditional index alongside retained full-scan oracle; differential tests | Equivalent output through candidate changes/Nop/executed retention; no state pruning | `protocol: index conflicts against the full-scan oracle` | 2026-10-07T13:20:00-04:00 | Conditional; not implemented |
| C107 | Validate a measured improvement before retaining the index | C106; M34 (C105) | conditional comparison workloads/profile analysis and report | Same workloads/environment, measured baseline vs index, disclose cost/regressions; otherwise omit M35 | `bench: validate conflict-index performance against baseline` | 2026-10-07T17:05:00-04:00 | Conditional; not implemented |

### M36: CI fault coverage and operating guide — 5 commits

Milestone prerequisites: M33 complete (C100); M34 complete (C105). Completion anchor: C112.

| ID | Implementation objective | Required dependencies | Expected source changes | Testing / acceptance | Proposed commit message | Proposed author = committer timestamp | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C108 | Run bounded failure suites on pull requests | C105; C107 only if M35 retained; M33 (C100), M34 (C105) | CI PR fault/test selection and regression checks | Deterministic small budget with replayable failures; existing sanitizer configuration reused | `ci: run bounded distributed fault regressions` | 2026-10-12T10:15:00-04:00 | Planned; not implemented |
| C109 | Run longer manual and scheduled exploration | C108; M33 (C100), M34 (C105) | nightly/manual failure matrices and budget controls | Explicit bounds/seeds and timeouts; no unbounded CI run or implied proof | `ci: schedule extended recovery exploration` | 2026-10-12T14:00:00-04:00 | Planned; not implemented |
| C110 | Retain failure artifacts and reproducible commands | C109; M33 (C100), M34 (C105) | CI artifact upload/retention and replay instructions | Saved traces/logs expose seed, version and failing command; retrieval/replay path checked | `ci: preserve replayable distributed failure artifacts` | 2026-10-12T17:50:00-04:00 | Planned; not implemented |
| C111 | Validate supported Windows compiler builds | C110; M33 (C100), M34 (C105) | Windows CI build/test job and platform fixes if necessary | Actual supported compiler coverage; no weakened checks or claimed unrun result | `ci: verify supported Windows builds` | 2026-10-14T11:05:00-04:00 | Planned; not implemented |
| C112 | Document tested operation and recovery limits | C111; M33 (C100), M34 (C105) | operator/developer guide and exercised runbook examples | Local deployment, failure recovery, known limits and benchmark reproduction match measured/tested behavior | `docs: document tested operation and recovery limits` | 2026-10-14T16:35:00-04:00 | Planned; not implemented |

## Approval and scope boundary

This revision supersedes every previous timestamp proposal, including all 2025
dates. No approval is inferred from a previous plan or the reviewed source trees.
Approve the full calendar and decomposition before committing or implementing the
remaining work. A future code change that cannot meet a row's acceptance criteria
does not earn a commit merely because a date is available. Any material split or
date change after approval must be surfaced explicitly.

Only planning/evidence documents change in this task. Do not create commits,
modify dates in Git metadata, reset/rewrite the working tree, configure a remote,
push, or start milestone 7. Preserve all existing snapshots and validation artifacts.

## Milestone 9 completed locally — 15 October 2026

| Slice | Commit | Author = committer timestamp | Full Debug / Release |
| --- | --- | --- | --- |
| C012 | `1caf07efe8fa053358ef2f37d04a54c216cf27fb` | 2026-07-07T17:10:00-04:00 | 104/104 each |
| C013 | `3f300f0b7c310845846e30d1b24d233d7e96da46` | 2026-07-10T09:50:00-04:00 | 114/114 each |
| C014 | `32e0c855d96af7154872d9e27352d2c7d3219005` | 2026-07-10T12:10:00-04:00 | 122/122 each |

Each exact staged export passed both configurations with -Werror before commit.
Final post-commit Debug and Release suites pass 122/122 each. C001–C011 remain
unchanged. Initial submission, receive and immutable duplicate replies are now
implemented; PreAcceptOK collection and quorum agreement remain unsupported.
Milestone 10 (C015–C018) is dependency-ready but not started. There are 98 remaining
planned slices, including two profiling-conditional optimizations, not a quota.
This actual October status and calendar evidence remain uncommitted. See
verification.md and preaccept.md; no remote or push was configured or performed.

## Publication and pause authorization — 15 October 2026

The user subsequently authorized publishing the existing 14-commit main branch
and an actual-date checkpoint/milestone-9 documentation branch to the public
anishsbala/Egalis repository. Earlier no-push statements record earlier approval
boundaries, not the current publication authorization. No engineering schedule,
source boundary or historical commit is changed. Milestone 10 remains unstarted.
The checkpoint is outside C001–C112 and must not be merged into main's approved
chronological sequence. See PAUSE_POINT.md for rebuilding and resuming.
