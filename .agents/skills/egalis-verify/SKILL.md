---
name: egalis-verify
description: Verify Egalis Debug and Release builds, complete deterministic tests, staged source and CI evidence without changing product code.
---

Read AGENTS.md and the task acceptance criteria. Inspect changes and CMake/CI
before selecting focused failure/replay checks. Use scripts/verify.ps1 in the
control checkout with -SourceDir pointing to the engineering worktree. Supply tool,
compiler and installed GoogleTest paths if needed; do not copy build scripts.

Implementation iteration uses one incremental build per source/toolchain/config,
focused RED/GREEN and affected regressions. Broaden on relevant changes/failures.
Do not duplicate a working full suite just before this full gate without a specific
reason. Protocol-safety slices retain fresh isolated exact staged Debug/Release.

For a commit gate, create a new ignored directory and use git checkout-index
--all --prefix=<absolute-export>/ from the intended index, then run the same
verifier with -SourceDir <export>, -SourceTree <git-write-tree>,
-SourceRevision <parent-commit>, and a separate -BuildRoot. Do not commit until this
exact tree passes; record git write-tree. Verify the committed tree equals it.
Never use a dirty working build as evidence for a different staged tree.

Reuse pinned installed dependencies via -GTestPrefix and optional compiler content
caching via -CompilerLauncher. Record launcher content digest and cache state (version may be unknown);
no unsafe sloppiness or shared mutable CMake directories across trees/toolchains/
configurations/sanitizer modes. Cache hits do not replace tests. Review may overlap
verification on the frozen snapshot. After PASS, root rechecks source tree, parent,
toolchain/options/dependencies and required suite; unchanged valid evidence needs
no redundant precommit rerun. Changes invalidate affected evidence. Preserve failed
runs; separate host/tool gaps from measured work and never relax checks/timeouts.

Run complete Debug and Release with warnings as errors and CTest zero-test
failure, plus task-specific deterministic simulations. Investigate command
failures and unexpected missing/skipped tests; do not silently reuse old logs.
The independent verification agent reports actual commands, time, revision/tree,
counts and paths to evidence. Findings go to root for repair. Run CI checks with
gh pr checks and inspect failed job logs; local success does not imply CI success.
Passing tests support the tested scope, not formal safety or durable restart.
Verifier command clocks are automatic. Record outer handoff/review/Git/CI phases
with scripts/workflow_timing.py, including source/parent IDs and overlap limits.
