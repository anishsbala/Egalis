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
