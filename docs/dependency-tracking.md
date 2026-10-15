# Known-conflict dependency oracle

Authority: [EPaxos* v1, Section 3.2, Figure 3 lines 3 and 9](https://arxiv.org/html/2511.02743v1#S3.SS2).
The scan returns every other known instance whose current cmd conflicts with the
queried application command. It reads the authoritative Replica instance table
directly, without a second table, cached candidate copy, phase/ballot filter, or
pruning. Committed records remain visible; a future execution marker must not
make the scan skip them. Self IDs are excluded per the existing message contract.

scan_known_dependencies is a pure, independently testable function over the
existing table type; Replica::known_dependencies delegates to it on its live map.
It returns an owning, numerically ordered set. Queries do not allocate instance
records or alter payloads, metadata, timers, phases, ballots or effects. Read-only
inspection also works while the core is suspended. The query ID need not exist.

The existing KV relation supplies bottom/Nop semantics: absent payloads conflict
with nothing; Nop conflicts with every real command, irrespective of key. The
Payload/Command overload avoids allocating/copying query strings during a scan.
Only result-set nodes are allocated. Complexity is linear in record count plus
the existing command-key comparisons; ordered insertion uses an end hint.

This milestone does not invoke preaccept, emit votes, initialize initial metadata,
or mutate stored dependency proposals. Those transitions remain milestone 9.

## Proposal expansion and candidate visibility

expand_known_dependencies and Replica::expand_dependencies return the Figure 3
line 9 union: proposed dependencies plus the current known-conflict scan. They
preserve proposed IDs even if unknown locally or currently nonconflicting. They
reject a proposed self dependency instead of silently dropping it. This is a value
computation only, not preaccept; neither the input proposal nor stored dep/initDep
is modified, and transitive closure is not computed here.

The dependency predicate remains current cmd, not initCmd. Initial metadata is
retained unchanged for future recovery-specific predicates; it is not a second
payload index. [Figure 9, Invariant 6](https://arxiv.org/html/2511.02743v1#A4.SS1)
states that after preaccept/validation the current payload remains the original
command or becomes Nop. Nop's conflicts therefore keep the instance visible even
to commands on unrelated keys. A validated initial-phase record with a real cmd
is visible too; untouched bottom records are not. No phase/ballot filter, cached
scan, or pruning may erase this knowledge. Keeping initial metadata does not mean
unioning initial dependency sets into every query or substituting initCmd for cmd.

Cross-phase fixtures describe records only, not implemented recovery transitions.
They verify that querying does not alter command, initial metadata, dep, phase,
ballots or timer guards. Integration tests query the same Replica through simulated
endpoints under ties, duplicates, loss, partitions, crash/restart and replay. Peer
PreAccept messages now consume this oracle in milestone 9: the first receive
unions the original proposal with current conflicts, then freezes its response.
Exact duplicates do not rescan. Other consensus messages remain unsupported;
recovery and Accept candidate changes are still deferred. See preaccept.md.
