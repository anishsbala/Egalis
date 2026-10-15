# Egalis

A C++20 distributed key-value store exploring baseline EPaxos*. **Work in progress:**
this is a protocol core and deterministic simulator, not a usable database server.

Status: milestones 1–9 implemented and locally tested. Checked protocol
types, deterministic KV semantics, baseline message contracts, and a single-threaded
simulator with fault injection, shared replica dispatch, known-conflict dependency
queries, initial PreAccept and trace replay are present. Quorum agreement, recovery,
persistent storage, TCP servers, and benchmarks remain unimplemented.

Development is paused after milestone #9. Debug and Release each pass 122 tests.
See [the pause point](docs/PAUSE_POINT.md) for the exact revision, limitations and
Windows rebuild/resume instructions. `main` preserves the 14 engineering commits;
`checkpoint/milestone-9` adds current documentation without changing that history.

- [Architecture, protocol selection, assumptions, and open decisions](docs/design.md)
- [Dependency-ordered milestones](docs/roadmap.md)
- [Testing strategy](docs/testing.md)
- [Shared replica dispatch and simulator runner](docs/replica-dispatch.md)
- [Initial PreAccept and duplicate-response policy](docs/preaccept.md)
- [Known-conflict scans and dependency proposal expansion](docs/dependency-tracking.md)
- [Verification record](docs/verification.md)
- [Six-batch commit plan and proposed calendar](docs/commit-plan.md)
- [Baseline transition contract](docs/baseline-transitions.md)
- [Simulator boundaries and fault semantics](docs/adr/0001-simulator-boundaries.md)

Requires CMake 3.24+, a C++20 compiler, and Git. Tests use an installed GoogleTest
1.15+ package or fetch a pinned GoogleTest revision during configuration.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Python tools will require Python 3.12+ when introduced. Protobuf, Docker Compose,
and server dependencies will be added with their first working consumers.

`main` contains fourteen verified engineering commits using approved reconstructed
July 2026 timestamps, not actual development dates. October planning evidence and
verification records are preserved on the checkpoint branch with an actual-date
documentation commit. Publication of these two branches is authorized; subsequent
publication or development requires a new request. No correctness or production
readiness guarantee follows from the tests, and no benchmarks have been measured.
