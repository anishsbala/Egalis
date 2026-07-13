# Frozen slow proposal

Authority: baseline EPaxos* arXiv2511.02743v1 Section3, Figure3 left12–18.
C019 consumes the sealed initial quorum; Accept receive and its acknowledgments
remain unimplemented until the following slices.

InitialSelection records collecting, fast or slow explicitly. A selected fast
quorum cannot authorize a slow continuation, even before immediate self-Commit
executes. SlowPathReady is an owned local effect and typed local event, never a
peer message. The runner records the effect and immediately dispatches the local
event through its existing effect-frame stack, before any later external input.
There is no reentrant core mutation or recursive effect pump.

The core accepts a continuation only for its locally owned, slow-selected,
ballot-zero preaccepted collection and the exact frozen sender/dependency map.
It unions every frozen response once, including a fourth mismatching response,
and freezes ballot, command and union in the instance's transient SlowProposal.
The proposal is separate from the authoritative local preaccept vote and initial
metadata; neither is overwritten to prepare the broadcast. No second instance
table or synthetic local acceptance is introduced.

All proposal and effect construction precedes publication. Publication precedes
execution of the self-first Accept broadcast to all five replicas. Duplicated,
tampered, unknown, foreign, superseded or fast-selected continuations cannot
start another broadcast. Later PreAcceptOKs cannot modify the frozen proposal.
Suspend/resume preserves a selected proposal and does not rebroadcast it; this
is volatile retained-memory lifecycle behavior, not durable restart.

With three responders the grace firing itself leads to Accept, without another
peer/client input. C019 deliberately leaves Accept receipt unsupported, so it
does not establish slow agreement, commitment, execution or client completion.
Deterministic tests check union/guards and the real runner's automatic broadcast,
including captured-trace reconstruction and replay. They are bounded regressions,
not a distributed-system safety proof.
