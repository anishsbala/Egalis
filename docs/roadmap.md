# Implementation roadmap

Each row is an engineering milestone that may span multiple coherent commits,
not a calendar commitment or claim of completion. At this source boundary, milestones 1–5 are implemented and locally verified,
at this boundary. Remaining milestones are planned. See commit-plan.md for status. Later work
starts only when prerequisites are verified. Small adjustments or split commits
are welcome when review reveals additional obligations; never manufacture changes
to meet a commit quota.

Every implementation commit must configure, build, pass existing tests and its
new behavioral tests, and update the design mapping when necessary. Partial
components are tested in isolation and are not advertised as a usable cluster.

| # | Proposed commit | Prerequisites | Functionality and acceptance evidence |
| --- | --- | --- | --- |
| 1 | build: establish C++20 and GoogleTest harness | none | Design documents, compiler contract test, CTest discovery, GCC/Clang CI definition; actual local build required before commit. |
| 2 | protocol: define baseline instance and ballot types | 1 | Checked ordering, ownership, sentinel separation, membership and quorum validation; boundary tests. |
| 3 | kv: define deterministic commands and conflicts | 2 | GET/PUT/CAS semantics, missing values, Nop and absent payload rules; exhaustive small-state commutativity tests. |
| 4 | protocol: extract baseline transitions and messages | 2,3 | Reviewed baseline-only transition document and typed messages, guards mapped to paper figures; exhaustive message field validation tests. |
| 5 | sim: add deterministic network scheduler | 4 | Logical-time event queue and explicit tie ordering; replay ordering tests without a consensus implementation. |
| 6 | sim: add fault schedules and trace replay | 5 | Delay, duplicate, drop, partition, crash-stop, timer events and versioned decision traces; identical replay artifacts. |
| 7 | protocol: introduce replica event dispatch | 4,6 | One authoritative instance table, explicit effects and immediate self-delivery; handler ordering and stale-event tests. |
| 8 | protocol: track known command dependencies | 3,7 | Full-scan dependency oracle with Nop handling; conflict visibility and insertion-order tests. |
| 9 | protocol: handle initial preaccept votes | 8 | Initial metadata, phase guards and distinct-sender replies; duplicates and reordered submissions. |
| 10 | protocol: implement baseline fast commit | 9 | Four equal-to-initial votes, frozen proposal and grace policy; thresholds and forbidden fast commits. |
| 11 | protocol: implement slow accept and commit | 10 | Union proposal, three accepts and ballot guards; disagreement, two failed peers, stale Commit tests. |
| 12 | sim: check agreement and visibility after events | 11 | Cross-replica observer over seeded concurrent histories; stores minimal failing trace. No liveness claim yet. |
| 13 | execution: construct committed dependency closure | 11 | Missing vertex handling, Nop vertices and blocked transitive dependencies; closure fixtures. |
| 14 | execution: order strongly connected components | 13 | Dependency-first SCC scheduling and fixed ID ordering; cycles and shuffled representation tests. |
| 15 | kv: apply replicated commands after execution | 3,14 | Real in-memory application, once-per-instance execution and results after apply; replica history comparison. |
| 16 | protocol: add recovery ballots and snapshots | 11,12 | Higher owned ballots, current/accepted separation, frozen three-replica snapshot; competing recoverers. |
| 17 | protocol: recover highest accepted decisions | 16 | Accepted/committed branch precedence and value preservation; coordinator crash at each slow-path stage. |
| 18 | protocol: validate possible fast decisions | 17 | Baseline candidate predicate and complete Validate/ValidateOK processing; paper Example 6 regression. |
| 19 | protocol: resolve invalidators and waiting cycles | 18 | All baseline waiting outcomes, Nop decisions and watches; mutual recovery blocking and late-event cases. |
| 20 | protocol: schedule recovery and owner resubmission | 19,15 | Explicit convergent detector inputs, increasing retries, missing-dependency recovery, Nop owner retry; quiescent completion under up to two failures. |
| 21 | test: explore bounded recovery interleavings | 20 | Small exhaustive scheduler plus seeded five-node suites; prune equivalent schedules with documented bounds and save counterexamples. |
| 22 | storage: add framed checksummed write-ahead log | 21 | Versioned records, flush failures and torn-tail handling; real temporary-file tests. Interior corruption fails closed. |
| 23 | storage: gate votes on durable protocol state | 22 | Persist all supporting protocol fields before dependent effects; crash cuts around every vote/validation mutation. |
| 24 | storage: replay identity and instance allocation | 23 | Stable configuration, checked ID reservations and replay equivalence; no ID reuse after crash. Restart review ADR required. |
| 25 | storage: atomically persist application results | 24,15 | Recoverable KV state, execution marker and result in one apply record; crash before/after flush and client reply. |
| 26 | client: deduplicate retries across instances | 25,20 | Replicated request/results table, request-aware conflicts, mismatch rejection and owner retry integration; uncertain response/retry histories. |
| 27 | storage: verify repeated crash and rejoin behavior | 26 | Same core in simulated durable runner; lagging replicas, obsolete in-flight messages, repeated crash cuts. Publish limits of restart refinement. |
| 28 | transport: introduce versioned protobuf codecs | 27 | Schema and typed-message adapters with size/identity validation; malformed, oversized and round-trip fixtures. |
| 29 | transport: add framed peer connections and retries | 28 | Chosen socket library, reconnection, bounded queues, eventual delivery policy and timer adapter; loopback integration tests. |
| 30 | server: run durable replicas on shared core | 29 | Working node executable, static configuration and detector policy; real multi-process agreement/failure tests. |
| 31 | client: expose key-value request and result API | 30,26 | CLI/client transport, request IDs, bounded errors and outcome-unknown timeouts; GET/PUT/CAS histories across reconnects. |
| 32 | deploy: add five-replica compose cluster | 31 | Pinned image build, durable per-node volumes, readiness and stop/restart workflow; automated compose smoke run. |
| 33 | test: check client histories for linearizability | 31 | Python 3.12+ bounded checker with valid/invalid oracle fixtures, pending calls, CAS and duplicate identities; real and simulated traces. |
| 34 | bench: add reproducible workload and trace analysis | 32,33 | Seeded uniform/hot-key workloads, offered/completed throughput, latency distributions, fast/slow/recovery counts; raw artifacts and environment metadata. No invented numbers. |
| 35 | protocol: index conflicts against full-scan oracle | 34 | Profile-driven indexing only if measurements justify it; differential tests and before/after measurements. If unnecessary, omit this commit. |
| 36 | ci: automate fault and sanitizer regressions | 33,34 | Bounded PR suites, longer manual/nightly runs, ASan/UBSan and artifacts; operating guide with demonstrated limits. |

Recovery milestones 16–20 form an incomplete subsystem until all five land; do
not label individual branches complete recovery. Networking depends on durable
fault-tested behavior so it cannot conceal unresolved simulator failures.

Excluded for this first scope: membership changes, sharding, snapshots, protocol
state pruning, Byzantine tolerance, transactions across keys, optimized EPaxos*,
thrifty fast paths, and unaudited local reads. Each needs a separate design.

Git identity is verified. The proposed calendar in commit-plan.md awaits approval. Commit
only functioning, reviewed milestones after that agreement. Do not rewrite an
existing repository or push without approval. The specification was published
4 November 2025; historical metadata must not imply this EPaxos* design predates
its reference or establish a false record of engineering dates.
