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
