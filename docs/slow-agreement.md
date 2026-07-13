# Frozen slow proposal

Authority: baseline EPaxos* arXiv2511.02743v1 Section3, Figure3 left12–18.
C019 consumes the sealed initial quorum; C020 implements Accept receipt and
its real acknowledgment. C021 collects distinct AcceptOKs and commits at three.

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
peer/client input. C020 installs the selected candidate on real self-Accept;
C021 commits after three distinct acknowledgments. Execution and client completion
remain outside this milestone.
Deterministic tests check union/guards and the real runner's automatic broadcast,
including captured-trace reconstruction and replay. They are bounded regressions,
not a distributed-system safety proof.


## Accept receipt (C020)

The receive guard is exactly Figure 3 right 1: current <= incoming ballot and
(current != incoming ballot or phase != committed). It uses current bal, not
accepted abal. Successful receipt sets both ballots, installs the supplied
payload/dependencies without conflict expansion, and sets accepted phase.
Unknown instances materialize without inventing initial metadata and local IDs
advance the volatile allocator. Existing initCmd/initDep remain untouched.
In particular, Accept(0)'s selected-Q union can be smaller than an earlier local
preaccept expansion; preaccepted dependencies are not an immutable Accept value.

Identical accepted requests replay one matching AcceptOK as a transport retry
refinement. Contradictory same-ballot accepted or frozen coordinator proposals
are rejected atomically. Retained CommittedDecision is distinct from the mutable
candidate and diagnoses contradictions even after a higher-ballot same-decision
Accept returns the phase to accepted. This trusted-sender invariant diagnostic
does not replace the paper guard or impose blanket dependency monotonicity.
The common Commit path also checks retained decision evidence.

Fresh acceptance cancels generic and grace timers. Initial observations remain
owned evidence but cannot collect again in accepted phase. A matching local
slow proposal remains frozen for C021's future collector; a superseding ballot
clears that proposal so old AcceptOKs cannot inherit coordinator ownership.
There is no synthetic vote and AcceptOK remains unsupported in C020.


## Slow acknowledgment collection (C021)

An empty SlowVotes collector is published with the locally issued frozen
SlowProposal. Only a real AcceptOK delivery adds its sender; immediate self
Accept and AcceptOK use the same serialized runner path as remote traffic.
The initial coordinator, ballot-zero proposal, slow branch, exact current and
accepted ballot and accepted phase must match. Unknown or merely accepted
instances cannot collect. A superseding ballot clears collector ownership.
AcceptOK has no payload/dependency fields; the immutable proposal supplies value
identity, with a diagnostic check against the accepted candidate.

Duplicate senders replay without adding votes. Two distinct replies cannot
choose, regardless of packet duplication. At the third distinct sender the core
constructs one self-first Commit broadcast from the frozen proposal and seals
the collector before any effect executes. The shared exact-ballot Commit handler
records the decision. Fourth/fifth, delayed or duplicate replies cannot alter or
rebroadcast it, including before the self-Commit effect is delivered. Retained
initial metadata and decision evidence remain distinct from the collector.
This slice initiates no positive-ballot recovery or durable restart behavior.
