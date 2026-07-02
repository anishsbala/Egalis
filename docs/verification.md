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
