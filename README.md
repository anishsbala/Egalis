# Egalis

A planned C++20 distributed key-value store based on baseline EPaxos*.

Status: batch 1 (milestones 1–6) implemented and locally tested. Checked protocol
types, deterministic KV semantics, baseline message contracts, and a single-threaded
simulator with fault injection and trace replay are present. Agreement, recovery,
persistent storage, TCP servers, and benchmarks remain unimplemented.

- [Architecture, protocol selection, assumptions, and open decisions](docs/design.md)
- [Dependency-ordered milestones](docs/roadmap.md)
- [Testing strategy](docs/testing.md)
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

Git identity is `anishsbala <anishsbala@gmail.com>` on `main`.
Commit timestamps are reconstructed metadata, not actual development dates.
No remote is configured. Pushing requires explicit approval.
