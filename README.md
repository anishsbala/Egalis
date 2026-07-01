# Egalis

A planned C++20 distributed key-value store based on baseline EPaxos*.

Status: milestone 1 source boundary. Build and design scaffolding only; no protocol or simulator functionality.
No consensus agreement, recovery, durable storage, TCP server, or benchmark is implemented.
This boundary's original test result was 1/1; fresh reconstruction checks
are retained in the separate history-preparation report.

- [Architecture, protocol selection, assumptions, and open decisions](docs/design.md)
- [Dependency-ordered milestones](docs/roadmap.md)
- [Testing strategy](docs/testing.md)
- [Verification record](docs/verification.md)

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
