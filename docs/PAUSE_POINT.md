# Egalis pause point — 15 October 2026

Development stopped after milestone **9: initial PreAccept**. Do not infer that
later roadmap milestones are implemented. This branch is an actual-date
documentation checkpoint, not part of the reconstructed engineering calendar.

Repository: https://github.com/anishsbala/Egalis (public).
Engineering branch: `main`, exactly 14 commits, ending at
`32e0c855d96af7154872d9e27352d2c7d3219005` (C014).
Documentation branch: `checkpoint/milestone-9`, based directly on C014.
No source, test, CMake or CI changes are introduced by the checkpoint.

## Completed history

| Milestone | Slice | Commit |
| --- | --- | --- |
| 1: build harness | C001 | e8b839db2e984c2338050538295a8ac4aae2c607 |
| 2: protocol types | C002 | 1f5145eb7239bdf4063c582139c09b7bacdafaed |
| 3: KV commands/conflicts | C003 | 86059f0f6d4cffc58b0e32fc176ddf530dfffe98 |
| 4: baseline messages | C004 | 8675efee39b2aa7739658d1f211245ac872a023d |
| 5: deterministic scheduler | C005 | 205c305089c7a8188c5671f9e50f2f6bcb4e91b3 |
| 6: network faults/replay | C006 | cdd8ef48ce2dd692ef48aaa35daa730cc6ae7444 |
| 7: shared event/effect dispatch | C007 | 3f51749ffe94f2c2afae48d8a2b2362188af90de |
| 7: serialized replica state | C008 | 0d5b1a1a7d968e361d814733c37f7378c6062f81 |
| 7: typed simulated endpoints | C009 | ccc4c98b11d74385a04cf41e136dfd7d6c19f2a2 |
| 8: known-conflict oracle | C010 | b58c9d98aa83811cb88466eb1f556c063cbbbc71 |
| 8: dependency expansion | C011 | 5be893c06070e69ac4304ee89ab3c07f4dc70db7 |
| 9: checked submission/allocation | C012 | 1caf07efe8fa053358ef2f37d04a54c216cf27fb |
| 9: guarded initial PreAccept | C013 | 3f300f0b7c310845846e30d1b24d233d7e96da46 |
| 9: frozen response replay | C014 | 32e0c855d96af7154872d9e27352d2c7d3219005 |

Original hashes, parents, trees and author/committer timestamps must not change.
Their July timestamps are intentionally reconstructed calendar metadata, not
claims about the actual engineering dates. The 112-slice plan is not a quota;
two optimization slices remain conditional on profiling evidence.

## Verification and limits

On 15 October 2026, before publication, both existing GCC 14.2 / Ninja builds
completed successfully with warnings treated as errors. CTest passed **122/122
Debug** and **122/122 Release** (3.06s and 2.55s measured runtimes respectively,
not performance benchmarks). No tests were skipped. C012–C014 also passed isolated
staged-tree Debug/Release gates, documented in [verification.md](verification.md).
Local build and validation artifacts/backups stay under ignored `build/`.

Implemented: checked C++20 types, deterministic KV semantics, message contracts,
single-threaded logical-time simulation, seeded randomness, fault injection,
trace serialization/replay, shared replica dispatch, conflict/dependency tracking,
initial PreAccept and exact duplicate-response replay. The same Replica core is
used by simulation and is intended for future real transports.

Not implemented: PreAcceptOK quorum collection, fast/slow agreement, dependency
graph execution, durable protocol/state-machine storage, recovery, production TCP
servers, deployment, or benchmarks. No complete EPaxos* consensus or safety/liveness
proof is claimed. Crash/resume retains volatile memory; it is not durable restart.
Exact retries replay the frozen initial response; future handlers must preserve
that invariant and count distinct senders rather than duplicated reply packets.
The selected reference remains [EPaxos* v1, Section 3](https://arxiv.org/html/2511.02743v1#S3).

## Rebuild and test in PowerShell

For the existing local environment:

```powershell
Set-Location 'C:\Coding Projects\Egalis'
$env:PATH = 'C:/msys64/ucrt64/bin;' + $env:PATH
& './build/tooling/cmake/data/bin/cmake.exe' --build build/batch2-debug --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/batch2-debug --output-on-failure
& './build/tooling/cmake/data/bin/cmake.exe' --build build/batch2-release --parallel 2
& './build/tooling/cmake/data/bin/ctest.exe' --test-dir build/batch2-release --output-on-failure
```

For a fresh clone, install Git, CMake 3.24+, Ninja and a C++20 compiler; put their
executables on PATH. The following assumes MSYS2 UCRT64 GCC at the stated path.
CMake uses installed GoogleTest 1.15+ or fetches its pinned revision (network
required). Tooling, downloaded dependencies and binaries are not in Git.

```powershell
git clone https://github.com/anishsbala/Egalis.git
Set-Location Egalis
$env:PATH = 'C:/msys64/ucrt64/bin;' + $env:PATH
foreach ($config in @('Debug', 'Release')) {
    $folder = 'build/verify-' + $config.ToLower()
    cmake -S . -B $folder -G Ninja "-DCMAKE_BUILD_TYPE=$config" `
        -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe -DCMAKE_CXX_FLAGS=-Werror
    if ($LASTEXITCODE -ne 0) { throw 'Configure failed' }
    cmake --build $folder --parallel 2
    if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
    ctest --test-dir $folder --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
}
```

## Resume milestone 10 later

Read this branch's design, roadmap, commit plan, testing, baseline transitions,
replica dispatch, dependency tracking, PreAccept notes and verification first.
Next dependency-ready work is milestone 10, **C015–C018**, in that order: distinct
observations, baseline fast commit, deterministic grace policy, and race tests.
Use the exact approved messages/timestamps from the checkpoint's commit plan.
Do not implement recovery or slow agreement ahead of its dependencies.

Return to `main` without merging/cherry-picking the October checkpoint commit:

```powershell
git status --short  # resolve/preserve any later local edits before switching
git switch main
git rev-parse HEAD # must initially equal C014 above
git log --oneline --reverse main
# Read the preserved plan without moving main:
git show checkpoint/milestone-9:docs/commit-plan.md
git show checkpoint/milestone-9:docs/PAUSE_POINT.md
```

The checkpoint retains all October planning, calendar evidence and actual-date
verification. Keep it available with git show or in a separate worktree; do not
bulk-stage its dated records into July engineering commits. Each future slice
must independently build/test, be reviewed, use its approved reconstructed
timestamp, and preserve main's history. Update actual-date records separately.
Future pushes need authorization. No additional protocol work was done to publish
this checkpoint.
