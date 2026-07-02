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
