# ADR 0001: deterministic simulator boundaries

Status: accepted for batch 1. This records engineering choices, not additional
EPaxos* rules. The selected protocol remains baseline Section 3.

## Scheduler and ownership

Simulation time is an unsigned 64-bit count of abstract ticks. Duration is a
different type; no wall-clock conversion is implied. Queue order is
`(LogicalTime, EventSequence)`. Sequence and EventId are separate types, allocated
monotonically per scheduler; cancellation never reuses them. An event scheduled
at the current tick joins after all previously scheduled events at that tick.
Counters and time arithmetic fail on exhaustion rather than wrap.

The ordered map owns callbacks and extracts each event before invocation, allowing
safe insertion and cancellation from callbacks. Cancellation scans the queue;
an extra index is unnecessary until measured workloads justify it. `run(limit)`
provides a deterministic event budget; zero-delay feedback can otherwise run
forever. No recursive `run`/`step` or concurrent access is supported. Observers
are instrumentation, cannot mutate the scheduler (enforced by a reentrancy guard),
and may throw to stop replay.
Callback/observer failures are terminal; continuation after a partial effect is
not supported.

Network owns endpoints through unique_ptr and owns its scheduler. It is nonmovable
so queued closures cannot invalidate its address. Destruction releases pending
callbacks before endpoints. No separate protocol algorithm exists in the simulator:
the Transport/Endpoint boundary exchanges protocol::Envelope and will connect to
the same replica core as the server at milestone 7. Current endpoint test doubles
exercise transport only, not consensus. Message IDs identify sends within one
network; duplicate copies share that ID but have distinct EventIds and copy indices.

Protocol self-delivery is immediate and cannot be delayed, dropped, duplicated or
partitioned, even with an explicit fault plan. All local consequences finish
before returning from send. Nested self-delivery follows the endpoint's call
stack; a future protocol dispatcher must prevent unbounded feedback. Remote
zero-delay sends still enter the scheduler. Endpoint exceptions and failures after
a send begins fail the network closed, without pretending the send was atomic.

## Fault semantics

FaultPolicy has inclusive minimum/maximum delays, integer probabilities out of
one million, and spacing between duplicate arrivals. Each remote send records
three choices: loss, duplication, delay. These choices are made even when loss
occurs. DeliveryPlan overrides randomness for that send and allows zero, one or
two copies with explicit delays from send time. Both arrival times are checked
before scheduling either copy. Overflow after a send begins is terminal.

`set_link` controls a directed pair. `partition(left,right)` changes both cross-group
directions only, in canonical replica order; overlapping/duplicate group members
are rejected before mutation. Existing unrelated link failures remain unchanged.
Packets blocked at send are lost immediately; healing cannot resurrect them.
In-flight packets check the link again at arrival. The simulator does not buffer
or retransmit dropped packets; reliable eventual delivery is a future effect-runner
obligation, not a property of this fault-injection transport.

A crash disables receives, sends and new timers from that replica. It advances
the destination generation. Every queued delivery/timer retains its original
generation, so a pre-crash event is discarded even after restart. A packet sent
while the destination is down can arrive after restart if it belongs to the new
generation; if it arrives while down it is lost. Already transmitted packets
remain deliverable after their source crashes. Missing destination endpoints are
recorded as undeliverable, and later attachment does not accept earlier packets.

Restart re-enables the existing endpoint; it does not erase/replay application
state or establish protocol crash-recovery correctness. Durable reconstruction
remains milestones 22–27. Tests explicitly distinguish crashed destinations from
stale generations and from partitions.

## Randomness, artifacts and replay

SplitMix64-v1 uses specified unsigned 64-bit arithmetic. Inclusive bounded draws
use rejection sampling, including singleton and full-width ranges; no standard
library distribution is involved. Golden outputs pin the algorithm. This is a
simulation PRNG, not cryptographic randomness.

Trace schema 1 stores PRNG version, seed, configuration, all scheduler decisions,
typed message fingerprints, fault choices/plans, link changes, lifecycle events,
deliveries/discard reasons, and timer disposition. Message fingerprint schema 1
encodes all fields in canonical order with length-framed byte strings. It is not
a network codec; Protobuf is still deferred. ASCII artifacts encode byte strings
as lowercase hex, with `-` for empty. Unknown versions, malformed integers/hex,
truncated fields and oversized records are rejected. A record supports up to 16
integer fields and 1 MiB of detail; traces are local trusted test artifacts, not
an untrusted network ingestion API. Total trace length is currently memory-bound.

Replay consumes the saved random decisions (it does not reroll them), checks
requested bounds/labels, and compares every other record exactly. Divergence
reports its decision index and permanently fails the trace. After draining the
queue, call `verify_replay_complete()` to detect unused records or unfinished
execution. Replaying an artifact still requires the same workload/scenario and
endpoint implementation; it is not a serialized C++ callback or process snapshot.
Application-state checking arrives with the shared core and history checker.

## Validation and remaining risks

Tests cover schedule ties, cancellation, bounded execution, overflow, ownership,
immediate self-delivery, all configured faults, destination epochs, callback
failures, seed repeatability, artifact round trips and replay divergence. Thirty-two
seeded multi-node fault scenarios replay in addition to fixed boundary fixtures.
Passing them does not prove consensus correctness. Remaining gates include the
core's duplicate-message behavior, timer token/ballot invalidation, durable state
cuts and eventual detector convergence; these belong to subsequent milestones.
