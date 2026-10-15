# Shared replica dispatch

Milestone 7 is runtime infrastructure, not an implementation of agreement.
The typed Event/Effect contract has no socket, scheduler or clock dependency.
Inputs own their payloads; effects are ordered values, not callbacks into the core.
Timer tokens, completion tokens, instance IDs and message IDs are distinct types.
Runners map abstract timer ticks to their configured time unit.

Submit explicitly supplies a locally owned unique instance ID. Durable allocation
is deferred to milestone 24. SubmissionRegistered only acknowledges local
registration, never commit, execution, or successful completion of a KV request.
Completion inputs are typed now but cannot resume anything until persistence and
application continuations exist. Their storage record and application-result
effects will be defined with those implementations, not guessed here.

Authority remains baseline-transitions.md and the pinned EPaxos* v1 design.
No protocol state guard or agreement rule is introduced by this contract.

## Authoritative state and dispatch

Replica owns one ordered instance map, exposed read-only. Submit reserves its
command in an initial local record without setting initial vote metadata or
emitting network traffic. Reused/foreign IDs are rejected rather than treated as
client retries. No extra mutable coordinator copy exists. Peer messages validate
their destination and structure, then return unsupported until their handlers
land. They do not create records or acknowledge votes.

Dispatch returns before any runner executes effects; the core accepts no callbacks.
All mutation, including timer registration and suspend/resume, is single-threaded.
One outstanding timer per instance is sufficient for this boundary. Replacement
returns cancellation before scheduling; a firing must match its instance, current
ballot and token, and is consumed once. A valid firing is explicitly unsupported
until an actual timeout policy exists. Unknown completions are stale because no
storage/application operation has been issued. Future continuations must register
their tokens and recheck ballots before resumption.

Suspend clears transient timers but retains the instance map and token allocator.
Resume cannot revive old timers. This is in-memory lifecycle isolation, NOT durable
crash recovery; reconstruction and durable ID reservation remain later milestones.

## Simulator runner

ReplicaEndpoint is created only through attach and owned by Network. Its core is
the same protocol::Replica a future server will instantiate. Timer closures cannot
outlive the endpoint: the owning network destroys the scheduler before endpoints.
Lifecycle hooks suspend/resume the core and cancel transient timer registrations;
in-flight network events retain the existing destination-generation protection.
Endpoint hook exceptions fail the network closed.

The runner finishes dispatch before running effects. It drains effect frames
iteratively; synchronous self-message results are pushed above the remaining
parent effects. Self-delivery therefore completes before the next effect without
recursive pumping. Remote messages still use the deterministic network queue.
Each pump has a configurable positive effect budget (default 65536); exhausting it
fails closed rather than hanging indefinitely on synchronous self-message feedback.
The configured budget is recorded in the trace and is a runner diagnostic limit,
not an EPaxos* transition rule or a claim of liveness.
execute exposes the runner boundary for transport conformance tests; it is not
an agreement handler, and test-supplied sends are not consensus decisions.
Unsupported messages, matched timers and unsolicited completions retain their
explicit status. Neither timer arming nor submission registration implies progress.

Trace schema 2 adds a protocol decision kind with canonical input, result and
effect records. Existing kind numbers and the PRNG remain unchanged; schema 1
artifacts remain readable and replayable for transport-only scenarios. They cannot
contain protocol records. Input fingerprints reuse the existing command byte
framing through a synthetic PreAccept representation solely as an encoding;
no PreAccept is sent or voted on. All tokens, ballots, completion tags, command
bytes and effect ordering participate in replay comparison. This is not a state
snapshot or a proof of agreement, and the same scenario/core version is required.

Once a dispatch or effect batch starts, unexpected failure is terminal for the
runner and network, rather than retrying a partially executed transition. Invalid
effect batches are rejected before starting. The single outstanding per-instance
timer is a mechanism; later milestones define its actual grace/recovery policy.

## Milestone 9 integration

SubmitCommand allocates a checked local ID; the trusted preallocated-ID Submit
uses the same coordinator path. Both freeze the initial proposal and emit
PreAccept to self before remote sends. Receiving PreAccept now installs the
initial vote and emits PreAcceptOK; exact retries replay the frozen reply without
another transition. Other peer messages remain unsupported, including vote
collection. Earlier descriptions of registration-only submission and unsupported
PreAccept describe the milestone 7 boundary, not current behavior. Trace input
encoding is still only framing, while emitted PreAccept messages are now real
protocol effects. See preaccept.md for guards, rejection and lifecycle limits.
