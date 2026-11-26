---
name: egalis-verify
description: Verify Egalis Debug and Release builds, complete deterministic tests, staged source and CI evidence without changing product code.
---

Read AGENTS.md and the task acceptance criteria. Inspect changes and CMake/CI
before selecting focused failure/replay checks. Use scripts/verify.ps1 in the
control checkout with -SourceDir pointing to the engineering worktree. Supply tool,
compiler and installed GoogleTest paths if needed; do not copy build scripts.

For a commit gate, create a new ignored directory and use git checkout-index
--all --prefix=<absolute-export>/ from the intended index, then run the same
verifier with -SourceDir <export>, -SourceTree <git-write-tree>,
-SourceRevision <parent-commit>, and a separate -BuildRoot. Do not commit until this
exact tree passes; record git write-tree. Verify the committed tree equals it.
Never use a dirty working build as evidence for a different staged tree.

Run complete Debug and Release with warnings as errors and CTest zero-test
failure, plus task-specific deterministic simulations. Investigate command
failures and unexpected missing/skipped tests; do not silently reuse old logs.
The independent verification agent reports actual commands, time, revision/tree,
counts and paths to evidence. Findings go to root for repair. Run CI checks with
gh pr checks and inspect failed job logs; local success does not imply CI success.
Passing tests support the tested scope, not formal safety or durable restart.
