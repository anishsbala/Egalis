# Egalis design proposal

Status: milestone 4 source boundary; consensus is not implemented.
Implemented scope: Foundation types, KV semantics, and the baseline-only message/transition contract. Decisions
below distinguish the selected algorithm from engineering extensions. Changes to
the algorithm require an explicit design revision and new counterexample tests.

## Specification boundary

Primary reference: Ryabinin, Gotsman, and Sutra, [Making Democracy Work: Fixing and
Simplifying Egalitarian Paxos, extended version, arXiv v1, 4 November
2025](https://arxiv.org/html/2511.02743v1). Pin v1 so that future revisions require
a documented comparison. Use Section 3, Figures 1, 3, and 5 with all highlighted
Section 4 additions removed. Appendix C supplies recovery initiation; Appendix D
is the invariant/proof reference. This is the baseline non-thrifty variant.

Configuration: n=5, f=2, e=1; the baseline bound
n >= max(2e+f+1, 2f+1) equals five. Fast quorums contain four distinct replicas;
slow and recovery quorums contain three. Counts include the coordinator's local
vote. Fast replies must equal the initial dependency proposal, not merely each
other. Recovery retains separate current and accepted ballots, validation, and
baseline Waiting handling. Never import original TentativePreAccept recovery.

Background only: Moraru, Andersen, and Kaminsky, [There Is More Consensus in
Egalitarian Parliaments, SOSP 2013](https://www.cs.cmu.edu/~dga/papers/epaxos-sosp2013.pdf).
Its dependency-oriented replication architecture motivates Egalis; its recovery
code, sequence-number tie breaking, and optimizations are not implementation
authority. Do not enable optimized or thrifty modes behind configuration flags.

## Assumptions and limits

The selected paper assumes non-Byzantine crash-stop failures, reliable links,
partial synchrony, deterministic commands, and eventual coordinator convergence.
Its liveness statement requires finitely many submissions. Safety must survive
arbitrary delay; progress requires a reachable surviving quorum. Our implementation
will test these conditions separately. No unconditional termination promise under
unbounded contention or permanent partition is intended. [Reference: Section 2
and Appendix C](https://arxiv.org/html/2511.02743v1#S2).

Engineering assumptions: static membership; authenticated/trusted replica
identities in the initial local deployment; no disk corruption, equivocation,
silent state loss, or counter wraparound. A replica with lost durable state cannot
rejoin under the same identity. Storage errors stop participation. Real transport
retries must implement eventual delivery once connectivity stabilizes; arbitrary
permanent packet drops are a safety test, not a liveness environment.

Crash/restart is an extension requiring a separate review of refinement to the
crash-stop algorithm. Every externally visible vote must follow durable state,
including preaccept and validation metadata. Stable replica identity and instance
allocation survive restart. Until that review and crash-cut tests pass, describe
the implementation as crash-stop only, even if the WAL can replay records.

## Component and event architecture

One serialized protocol event loop per replica. The core owns protocol state and
accepts typed inputs: client submission, peer message, timer firing, persistence
completion, and application completion. It emits explicit effects: send, schedule
timer, persist batch, apply committed work, and reply to client. These are proposed
interfaces, not implemented stubs. Production I/O workers enqueue completions;
they never mutate protocol state. No sockets, wall-clock reads, global random
generator, blocking filesystem calls, or thread scheduling inside the core.

The simulator and server instantiate the same core and executor libraries. Only
effect runners differ. The simulator controls logical time, delivery order,
storage completions, and failure events. Determinism requires canonical ordering
of sets, graph traversal, tie breaks, and recorded PRNG version/seed; a seed alone
is not a portable trace. Replay records the actual event decisions.

| Component | Owns | Does not own |
| --- | --- | --- |
| Protocol types | Strong IDs, ballots, commands, typed messages | Protobuf classes |
| Replica | Instance table and transition dispatch | Threads and sockets |
| Agreement | Transient per-ballot vote collectors | A second instance table |
| Conflict index | Derived lookup of known conflicts | Independent source of truth |
| Executor | Committed closure and execution scheduling | Consensus decisions |
| Storage | WAL framing, flush, replay, atomic apply records | Quorum policy |
| Simulator | Event queue, fault schedules, replay artifacts | Alternative consensus code |
| Server transport | Framing, connections, retries, bounded queues | Protocol transitions |
| Client adapter | Requests, completions, retry identity | Local-read shortcuts |

All state changes go through explicit transitions. A storage-backed runner gates
dependent effects on a durable batch completion. Pending continuations are
ballot-checked when resumed; stale storage/timer completions cannot revive a
superseded coordinator. Model self-delivery synchronously, including all local
consequences, before later external events. Define ordering between persistence
and self-delivery in the storage refinement rather than casually inserting an
asynchronous self-message into the simulated network.

## State ownership and invariants

Paper field names will remain visible in transition comments and a maintained
mapping table. Persistent per-instance data is planned to include bal, abal,
phase, cmd, dep, initCmd, and initDep. Pending vote collectors, recovery quorum
snapshots, invalidator watches, and timers are transient; reconstruct them through
recovery after restart. Retain initial metadata even when a candidate becomes Nop.
Do not duplicate accepted values in an independently mutable coordinator cache.

Proposed executable checks (cross-reference the exact predicates in Appendix D
before implementing each):

- Every replica's committed record for an ID has identical payload and dependencies.
- Conflicting committed application commands have a dependency edge between them.
- Current ballot never decreases; accepted ballot never exceeds current ballot.
- One coordinator owns each ballot. One proposal is frozen per instance/ballot;
  vote collectors accept each member at most once and reject mismatched ballots.
- Initial preaccept happens at most once. Recovery validation is not preaccept.
- Committed decisions never change. Local phase labels are not assumed to be a
  simple monotonic enum: a higher-ballot Accept may revisit accepted state.
- Application effects occur at most once and only from a committed dependency
  closure. Nop settles a protocol instance without applying a client operation.
- IDs and request identities never alias; counter exhaustion is a checked error.
- Durable votes cannot precede their supporting records; acknowledged execution
  includes durable application state and a recoverable result.

The first two checks express the paper's central safety properties. The remaining
checks are implementation obligations, not a replacement proof. Figure 9 also
needs transition-specific assertions; do not infer universal dependency monotonicity
from ballot monotonicity. [Invariant reference: Appendix D](https://arxiv.org/html/2511.02743v1#A4).

Identifiers will encode (counter, replica) in a fixed total order corresponding
to the numerical ID model. Ballot zero belongs to the initial coordinator;
positive ballots use a checked encoding of (round, coordinator). Decide widths
and durable allocation before exposing a wire schema.

## Execution and key-value semantics

The executor implements Figure 1 directly before optimizations. Explicitly test
edge direction: dependencies precede dependents. Canonical SCC traversal must
not depend on hash iteration. Missing dependency vertices block execution and
trigger recovery through the core. Keep executed graph metadata initially;
garbage collection is outside this roadmap until a separate safe pruning design.

Initial application: byte-string keys/values; GET, PUT, and CAS. Define missing
separately from an empty value. PUT returns acknowledgement after execution; CAS
compares an optional expected value and returns success plus the observed value.
All reads enter consensus. Different-key commands commute; same-key GETs commute;
any same-key pair involving a mutation is conservatively conflicting. Avoid
special-casing equal PUT values. Internal Nop is distinct from a user command and
follows the specification's conflict rules; absent payload is another distinct tag.

Client identity is separate from protocol instance identity. Begin with unique
requests and no transparent retries. Before automatic retry, add a replicated
request-result table and payload mismatch rejection. The conflict relation must
include shared request identity, even for different keys; otherwise duplicate
requests could execute in inconsistent orders. The table and state mutation need
one atomic durable apply record. Nop replacement gets a new instance while retaining
the logical request. A timeout means outcome unknown, not operation aborted.

## Hard questions and gates

| Question / gap | Required resolution before affected code |
| --- | --- |
| Figure 5 visually combines baseline and optimized code | Write a baseline-only transition table; remove highlighted clauses and arguments, including the Waiting threshold and extra late-response branch. Review against Section 3 prose and proofs. |
| When to stop collecting preaccept replies | Specify a deterministic fast-path grace policy. Taking slow path immediately at the third equal reply defeats the intended fast path. A conflict can trigger slow path; bounded grace permits progress with only three responders. Prove proposal is frozen once chosen. |
| Recovery quorum and invalidator observation | Freeze the recovery snapshot Q and its candidate. Validation waits for every member of that Q; missing responses cause a higher-ballot retry, not substitution or partial approval. Snapshot I separately from current local resolution watches. |
| Duplicate messages vs pseudocode preconditions | Define replay of identical replies without repeating state transitions; reject conflicting duplicates. Test retries after lost replies and superseded ballots. Do not broaden Commit's ballot guard. |
| Recovery ownership | Map eventual per-instance coordinator convergence to a concrete heartbeat/suspicion policy. Increasing retry delays and stale timer invalidation need traces; timeout alone is not permission to commit. |
| Validation persistence and phase behavior | Preserve the distinction between initial metadata, current candidate, and accepted ballot. Extract all branch guards before coding; uncertain guards remain unresolved design issues. |
| Restart and self-message atomicity | Specify durable cuts and continuations, then fault-inject each cut. Crash-stop proofs do not automatically establish restart safety. |
| Nop and client retry | Specify owner resubmission and cross-replica retry deduplication separately. Define request retention limits before claiming exactly-once behavior. |
| Retaining all known dependencies | Start with a full scan and no pruning. Compare an indexed implementation against that oracle; do not omit executed commands without a proof. |

These are implementation/refinement gaps, not claims that the baseline paper is
incorrect. If extraction exposes an actual contradiction, pause the affected
transition and document the competing interpretations rather than invent a rule.

## Proposed repository layout

Only files with present functionality exist initially. Create the following
directories with their first implementation, rather than filling them with stubs:

```text
Egalis/
  CMakeLists.txt
  cmake/                     # tooling and sanitizer options
  include/egalis/
    protocol/                # types, messages, replica interface
    execution/               # dependency graph and executor
    storage/                 # WAL and application state contracts
    sim/                     # deterministic runner
    transport/               # transport boundary
    kv/                      # command and result model
  src/{protocol,execution,storage,sim,transport,kv}/
  apps/{server,client,sim}/
  tests/{unit,integration,simulation,history}/
  tests/fixtures/            # immutable counterexample traces
  proto/                     # introduced with real transport
  tools/                     # Python 3.12+ trace and benchmark tools
  deploy/                    # functioning five-node Compose deployment
  benchmarks/                # workloads and measured artifacts
  docs/{design.md,roadmap.md,testing.md,verification.md,adr/}
  .github/workflows/
```

Target boundaries will follow dependencies: types -> protocol and execution;
storage and transports implement effect runners; simulator/server link the same
protocol. Keep dependencies private where possible. Pin third-party versions;
Protobuf enters only with a working codec and transport. Choose the socket library
in that milestone's ADR, after measuring actual interface needs.

## Decision record

Accepted for planning: C++20, fixed baseline configuration, shared event core,
serialized replica mutation, deterministic simulator first, execution-before-reply,
no pruning, and separate crash-restart review. Deferred: exact durable schema,
transport library, coordinator detector policy, retry retention, size limits,
and snapshots/reconfiguration. Each deferred choice gets an ADR containing source
sections, alternatives, invariants affected, and validation evidence before coding.

Do not infer correctness or production readiness from passing tests. Benchmark
claims require reproducible measured artifacts. Local Git initialization and the account-matching identity are authorized.
Historical schedule approval is pending; no commits, historical timestamps,
remote or push are authorized at this preparation stage.
