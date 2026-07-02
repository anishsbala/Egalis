# Testing strategy

## Current coverage

This boundary contains 30 GoogleTests, including the original C++20 build
contract. Implemented scope: Foundation/message contracts plus the logical-time scheduler and typed simulated transport.
Agreement/recovery and durable protocol behavior are not implemented or tested.

## Behavioral testing layers

Unit tests cover type boundaries and ballot ownership; conflict symmetry and
conservative classification; command results across all small KV states; quorum
sender identity; dependency closure; SCC ordering; and each transition guard.
Use tiny exhaustive application models to check that every pair classified as
commuting preserves both results and final state. False negatives in conflicts
are safety defects; false positives cost performance.

Transition tests enumerate each selected figure's handlers with precondition
true/false, duplicate and stale messages, missing fields, competing ballots, and
restart continuations. Maintain a source-to-handler-to-test table as handlers
are introduced. Test higher-ballot Accept after local commit without confusing
phase monotonicity with decision immutability. Freeze coordinator proposals and
quorum snapshots so late replies cannot silently change a chosen value.

Deterministic five-node scenarios cover four-vote fast commits, unequal initial
dependencies, slow agreement with three survivors, partitions, reordered messages,
coordinator crashes before and after every decision point, ballot races, missing
dependencies, SCC cycles, recovery invalidators, Nop resubmission, and blocked
recovery cycles. Transcribe the paper's Example 6 and Appendix E failure pattern
into fixtures, documenting adaptation to the selected baseline variant. Assert
that expected branches were reached; a scenario that never triggers recovery
is not a recovery test.

An observer checks committed agreement, conflict visibility, unique application,
and consistent ordering of conflicting application commands after every event.
Do not require identical intermediate KV states when replicas have executed
different sets. Compare converged states only after the same logical operations
are applied. Randomized exploration stores seed, generator version, configuration,
exact scheduler decisions, and first violated invariant. Replay does not depend
on wall-clock time. A minimizer removes events only when the same failure remains
and reports the reduced schedule as a fixture.

Bounded exploration uses small command sets, all quorum membership combinations,
and selected crash/delivery interleavings. State its bounds in results; passing
bounded exploration is not model checking of the full system or a formal proof.
Liveness runs stop submission, restore eventual delivery, converge the recovery
detector, and supply fair timer execution. Use logical event budgets to detect
stalls and preserve traces. Permanent minority partitions may prevent clients
there from progressing; do not mislabel that expected behavior as a protocol bug.

Storage tests inject failure before append, within a record, after append before
flush, after flush before send, after application flush before reply, and during
replay. Persist preaccept and validation state as well as accepted state. Check
stable identity, monotonic allocation, no duplicate apply, cached response recovery,
and fail-closed handling of corruption. Simulated storage supports these cuts;
real temporary-file tests verify framing/replay and OS flush error propagation.
Power-loss durability remains constrained by the filesystem/device contract.

Transport tests cover framing boundaries, connection loss, reconnection,
duplicates, malformed protobuf, bounded queues, oversized requests, membership
validation, and stale messages after restart. Real network tests use readiness
and observable events with bounded deadlines; fixed sleeps are not synchronization.
They supplement deterministic tests rather than define protocol correctness.

## Client histories

Record invocation and response events, request identity, payload, result, replica,
and monotonic timestamps. The checker searches sequential histories consistent
with completed-call real-time precedence and the KV model; pending requests may
have taken effect. CAS includes observed-value semantics. Treat retries as one
logical operation only after deduplication exists. Reject mixed payloads for the
same identity. Use hand-written valid/invalid histories to validate the checker
before trusting a cluster result. Keep global checks bounded; partition by key
only when request identities and supported operations justify independence.

## CI and measurements

Initial CI definition builds GCC and Clang on Linux and runs CTest. Add sanitizer
builds when behavioral code appears, and Windows compiler coverage after the first
supported local build. Bounded deterministic seeds run on pull requests; larger
fault suites run manually/nightly. Record exact commands, compiler/dependency
versions, test counts and failures. Never replace an unavailable dependency with
a fake test framework to obtain a green result.

Benchmark tools require Python 3.12+ and emit raw machine-readable samples with
source revision, compiler flags, host/container resources, topology, seeds,
warmup, duration, offered load, payload size, key distribution, retry count, and
error rate. Distinguish commit latency from execution/client latency. Report
throughput of completed logical operations separately from submission rate;
include tail latency and recovery stalls. Publish measured numbers only after
checking corresponding histories and disclosing checker limits.

## Milestone gate

Before every code commit: inspect the diff, configure/build with CMake, run the
relevant GoogleTest/CTest suites, preserve new counterexample fixtures, and update
the verification record. A blocked build remains a blocked verification step.
No claim of production readiness or formal correctness follows from passing tests.
