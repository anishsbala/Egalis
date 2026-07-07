# Boundary verification

This summarizes reconstructed source boundaries, not historical verification
sessions. Commit timestamps are calendar metadata, not test dates. Actual dated
execution logs are retained separately from these trees.

| Milestone | Verified CTest cases |
| --- | --- |
| 1 | 1/1 |
| 2 | 6/6 |
| 3 | 10/10 |
| 4 | 19/19 |
| 5 | 30/30 |
| 6 | 56/56 |

Each boundary is configured, built and tested in isolation with CMake/Ninja,
GCC, pinned GoogleTest and `-Werror`. The final boundary also requires Release
validation. CI and sanitizers are not claimed to have run. Passing tests do not
establish formal consensus correctness or production readiness.

```powershell
# Supply compiler, Ninja and GoogleTest paths when offline.
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=-Werror
cmake --build build/debug --parallel 2
ctest --test-dir build/debug --output-on-failure
```

## C007: explicit event/effect contract

GCC Debug with -Werror configured and built successfully; CTest passed 60/60.
Four new contract tests cover payload ownership, ordered effects, invalid tokens,
completion tags and delegation to message validation. No agreement handlers exist.
Actual dated logs are maintained separately; this is a reconstructed boundary summary.

## C008: serialized shared replica dispatch

GCC Debug with -Werror built successfully; all 67/67 CTest cases passed.
Seven new tests cover unique local registration, immutable authoritative state,
canonical map iteration, all eleven unsupported message routes, timer replacement,
ballot/token guards, stale completions and suspend/resume isolation.
No preaccept, agreement, recovery or storage transition is implemented.

## C009: shared core simulator endpoint

GCC Debug with -Werror built successfully; all 83/83 CTest cases passed.
Sixteen additional tests cover shared dispatch, synchronous self-effects, bounded
pumping, timer replacement, crash/restart, duplicates, effect-source validation,
fail-closed errors, callback lifetime, seeded replay and schema compatibility.
Sixteen seeded five-replica scenarios run twice and replay from serialized traces.
Trace schema 2 records core decisions; schema 1 transport artifacts remain readable.
Agreement remains unimplemented. Full post-commit Debug/Release verification is
recorded separately from reconstructed timestamp metadata.

Post-commit Release initially stopped under -Werror on a maybe-uninitialized
diagnostic in an intermediate effect-variant move. Constructing timer/cancellation
alternatives directly in the vector removed the unnecessary move without weakening
compiler flags. After this integration fix, Debug and Release both passed 83/83.

## C010: complete known-conflict scans

Full GCC Debug and Release suites with -Werror passed 91/91 each.
Eight new tests cover empty/unknown targets, KV conflicts, exact self exclusion,
all phases and ballots, retained committed commands, Nop/bottom, all 24 insertion
orders, allocation-free command queries and live-table ownership.
The read-only oracle follows EPaxos* v1 Figure 3 lines 3/9. No vote, preaccept,
state mutation, conflict index or pruning is introduced. The existing sixteen-seed
core replay and fault-injection suites remain included in the full test runs.
The exact staged tree is also required to pass isolated Debug/Release builds and
91/91 tests before this boundary is committed; actual dated logs are retained
separately from these reconstructed metadata dates.

## C011: dependency visibility across candidate changes

Full GCC Debug and Release suites with -Werror passed 99/99 each.
Six new unit cases verify proposal unions, unknown IDs, self-dependency rejection,
retained initial metadata through Nop records, validated initial-phase knowledge,
absence of transitive expansion, and unsupported candidate changes. Two simulator
cases cover equal-time knowledge ordering and eight seeded five-replica fault
scenarios, each repeated and replayed from serialized traces. Original protocol
metadata is preserved; cmd, not initCmd, remains the dependency predicate.
The exact staged tree must also pass isolated Debug/Release builds and 99/99 tests
before committing. No preaccept, accept, recovery or execution handler is added.
Actual dated verification is retained separately from reconstructed commit dates.

## C012: checked submission and allocation

Working Debug and Release builds with -Werror passed 104/104 tests each.
Five new cases exercise local allocation, reserved-ID rejection/high-water marks,
last-ID exhaustion, frozen proposals, suspend/resume and allocated-input trace replay.
Existing assertions now account for five PreAccept effects and frozen metadata;
no test case is removed. Receiver-side voting is still unsupported in this slice.
The exact staged export must pass both configurations before committing.
Actual dated execution records remain separate from reconstructed commit dates.
