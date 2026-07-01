# Boundary verification

This summarizes reconstructed source boundaries, not historical verification
sessions. Commit timestamps are calendar metadata, not test dates. Actual dated
execution logs are retained separately from these trees.

| Milestone | Verified CTest cases |
| --- | --- |
| 1 | 1/1 |
| 2 | 6/6 |

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
